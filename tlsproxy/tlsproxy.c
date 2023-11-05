#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tls_help/certs.h"
#include "mbedtls/build_info.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"

int main(int argc, char *argv[])
{
	const char *HOST, *CLIENT_PORT, *SERVER_PORT;
	int ret = 1, len;
	int exit_code = EXIT_FAILURE;
	mbedtls_net_context server_fd;
	uint32_t flags;
	unsigned char buf[1024];
	const char *pers = "ssl_client1";
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context server_ssl;
	mbedtls_ssl_config server_conf;
	mbedtls_x509_crt cacert;

	if (argc != 4)
	{
		fprintf(stderr, "usage: %s HOST PORT\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	CLIENT_PORT = argv[1];
	HOST = argv[2];
	SERVER_PORT = argv[3];

/**
 * BEGIN CONNECTION TO SERVER
 */
	/*
	 * 0. Initialize the random-number generator and the session data.
	 */
	mbedtls_net_init(&server_fd);
	mbedtls_ssl_init(&server_ssl);
	mbedtls_ssl_config_init(&server_conf);
	mbedtls_x509_crt_init(&cacert);
	mbedtls_ctr_drbg_init(&ctr_drbg);

	printf("\n  . Seeding the random number generator...");
	fflush(stdout);

	mbedtls_entropy_init(&entropy);
	if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers))) != 0)
	{
		printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	 * 1. Load certificates.
	 */
	printf("  . Loading the CA root certificate ...");
	fflush(stdout);

	ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
	if (ret < 0)
	{
		printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", (unsigned int)-ret);
		goto exit;
	}

	printf(" ok (%d skipped)\n", ret);

	/*
	 * 2. Start the connection.
	 */
	printf("  . Connecting to tcp/%s/%s...", HOST, SERVER_PORT);
	fflush(stdout);

	if ((ret = mbedtls_net_connect(&server_fd, HOST, SERVER_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
	{
		printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	 * 3. Setup configuration.
	 */
	printf("  . Setting up the SSL/TLS structure...");
	fflush(stdout);

	if ((ret = mbedtls_ssl_config_defaults(&server_conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	 * 4. OPTIONAL is not optimal for security,
	 * but makes interop easier in this simplified example
	 */
	mbedtls_ssl_conf_authmode(&server_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
	mbedtls_ssl_conf_ca_chain(&server_conf, &cacert, NULL);
	mbedtls_ssl_conf_rng(&server_conf, mbedtls_ctr_drbg_random, &ctr_drbg);

	if ((ret = mbedtls_ssl_setup(&server_ssl, &server_conf)) != 0)
	{
		printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
		goto exit;
	}

	if ((ret = mbedtls_ssl_set_hostname(&server_ssl, HOST)) != 0)
	{
		printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
		goto exit;
	}

	mbedtls_ssl_set_bio(&server_ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	/*
	 * 5. TLS handshake.
	 */
	printf("  . Performing the SSL/TLS handshake...");
	fflush(stdout);

	while ((ret = mbedtls_ssl_handshake(&server_ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int)-ret);
			goto exit;
		}
	}

	printf(" ok\n");

	/*
	 * 6. Verify the server certificate.
	 */
	printf("  . Verifying peer X.509 certificate...");

	/* In real life, we probably want to bail out when ret != 0 */
	if ((flags = mbedtls_ssl_get_verify_result(&server_ssl)) != 0)
	{
		char vrfy_buf[512];

		printf(" failed\n");
		mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
		printf("%s\n", vrfy_buf);
	}
	else
	{
		printf(" ok\n");
	}

/**
 * END CONNECTION TO SERVER
 */

/**
 * BEGIN CONNECTION TO CLIENT
 */
	short client_port = strtol(CLIENT_PORT, NULL, 10);
	if (client_port == 0)
	{
		fprintf(stderr, "usage: %s PORT\n", argv[0]);
		goto exit;
	}

	int sockfd, rc, connfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("error creating listening socket");
		goto exit;
	}

	// Setting client socket attributes
	struct sockaddr_in client_addr = {0};
	client_addr.sin_port = htons(client_port);
	client_addr.sin_family = PF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;

	// Binding
	rc = bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if (-1 == rc)
	{
		perror("error␣binding␣listening␣socket␣to␣port");
		goto exit;
	}

	// Listening
	rc = listen(sockfd, 0);
	if (-1 == rc)
	{
		perror("failed␣to␣listen");
		goto exit;
	}

	// Accepted connection
	connfd = accept(sockfd, NULL, NULL);
	if (connfd == -1)
	{
		perror("accept");
		goto exit;
	}

	// Receiving clients request
	unsigned char client_buf[1024];
	while (1)
	{
		ssize_t size = recv(connfd, client_buf, sizeof(client_buf), 0);

		/**
		 * Forward client's request to server
		 */
		printf("  > Write to server:");
		fflush(stdout);

		while ((ret = mbedtls_ssl_write(&server_ssl, client_buf, strlen(client_buf))) <= 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			{
				printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
				goto exit;
			}
		}

		len = ret;
		printf(" %d bytes written\n\n%s", len, (char *)client_buf);

		if (strstr("\r\n", client_buf) == 0)
		{
			break;
		}
	}

/**
 * END CONNECTING TO CLIENT, AND FORWARDING REQUEST
 */

	/*
	 * Read the HTTP response from the server
	 */
	printf("  < Read from server:");
	fflush(stdout);

	do
	{
		len = sizeof(buf) - 1;
		memset(buf, 0, sizeof(buf));
		ret = mbedtls_ssl_read(&server_ssl, buf, len);

		if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			continue;
		}

		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
		{
			break;
		}

		if (ret < 0)
		{
			printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
			break;
		}

		if (ret == 0)
		{
			printf("\n\nEOF\n\n");
			break;
		}

		len = ret;
		printf(" %d bytes read\n\n%s", len, (char *)buf);

		/*
		 * Send Servers responce back to client
		 */
		printf("  > Write to client:");
		fflush(stdout);

		ret = send(connfd, buf, sizeof(buf), 0);
		if (ret == -1)
		{
			printf("Send back failed\n");
		}
		len = ret;
		printf(" %d bytes written\n\n%s", len, (char *)buf);
	} while (true);

	mbedtls_ssl_close_notify(&server_ssl);

	exit_code = EXIT_SUCCESS;

exit:

	if (exit_code != EXIT_SUCCESS)
	{
		char error_buf[100];
		mbedtls_strerror(ret, error_buf, 100);
		printf("Last error was: %d - %s\n\n", ret, error_buf);
	}

	mbedtls_net_free(&server_fd);
	mbedtls_x509_crt_free(&cacert);
	mbedtls_ssl_free(&server_ssl);
	mbedtls_ssl_config_free(&server_conf);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	if (-1 != sockfd)
	{
		close(sockfd);
	}
	if (-1 != connfd)
	{
		close(connfd);
	}
	exit(exit_code);
}
