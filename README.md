# 140E final project
To run tests: 
```
cd demo
make
```
tests starting in "uart-test" will start running automatically.

To test router connection, note that you'll have to update the constants to include your SSID and router password.

# TODO
- [ ] implement sw-uart (currently using staff version)
- [ ] create wrapper code for AT commands (eg, getters and setters, formatting including CRLF)
- [ ] create reliable routines for connecting to APs
- [ ] figure out how to route data back to Pi from TCP connection
