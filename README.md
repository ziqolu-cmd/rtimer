# rtimer (C, oneshot)

This is a small oneshot timer utility implemented in C.

Usage:
  rtimer <seconds> [-- command [args...]]

Examples:
  rtimer 5
  rtimer 2.5 -- echo "Hello after 2.5s"

Behavior:
- Waits the specified number of seconds (supports fractional values).
- After the wait, if a command is given it executes it and exits with the command's exit code.
- If no command is given, exits 0 after sleeping.
