# 140E final project
To run tests: 
```
cd demo
make
```
tests starting in "uart-test" will start running automatically.

To test router connection, note that you'll have to update the constants to include your SSID and router password.

# TODO
- [x] implement sw-uart (currently using staff version)
- [x] create wrapper code for AT commands (eg, getters and setters, formatting including CRLF)
    - [x] port over code from 140E 2022 lab 16
    - [x] adapt 140E 2022 lab 16 code to use pi sw-uart interface instead of unix file descriptors
    - [ ] debug heap overflow issues in lex buffer
    - [ ] figure out reliable way to get number of bytes read in `esp_read` (see `libesp/pi-support.c:17`) instead of static buffer length
- [ ] create reliable routines for connecting to APs
- [ ] figure out how to route data back to Pi from TCP connection
