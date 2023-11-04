/* Either run locally, or share over network with xinetd. */
package main
import (
        "os"
        "fmt"
        "syscall"
)
const SECRET = "[Sanitized]"
func main() {
        var isSocket bool
        // Is stdin the real stdin or a socket?
        info, err := os.Stdin.Stat()
        if err != nil {
                panic(err)
        }
        if info.Mode() & os.ModeSocket == 0 {
                isSocket = false
        } else {
                isSocket = true
        }
        if isSocket {
                /*
                 * Get the address from the received message to avoid
                 * EDESTADDRREQ. Netcat does not require this, but xinetd does.
                 */
                // Don't care what message contains; need peer address.
                buf := make([]byte, 1)
                _, peer, err := syscall.Recvfrom(syscall.Stdin, buf, 0)
                if err != nil {
                        panic(err)
                }
                if err := syscall.Sendto(syscall.Stdout, []byte(SECRET + "\n"), 0, peer); err != nil {
                        panic(err)
                }
        } else {
                fmt.Print(SECRET + "\n")
        }
        os.Exit(0)
}