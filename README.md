# C program to control Wiz / Philips Wiz connected light

Wiz (Philips) smart lights works with raw UDP datagram on port 38899 without authentication, so if you are on the same network you can control any bulb with a simple program like this one. 

The datagram consist in a JSON object with a simple syntax, the main parameters are: 
 - method, that specify what the light have to do.
 - params, that specify parameters for the chosen method. 

An example is: 

``` json
"{"method": "getPilot", "params": {}}"
```

that allows you to get status information from the light. Some of the possible methods and parameters is definied as macro in the main.c file.

## Propgram description
You can launch the program in two ways: 
``` bash
./main 
```
or 
``` bash
./main 192.168.0.22
```

If you use the first way it ask you if you want to discover the bulbs presents on the network, in the oder way it gives you a prompt and you can launch some commands: 
``` bash
#first way
./main
No bulb on argv[1], do you want to discover? (y/n): y
Enter broadcast address: 192.168.0.255
Bulb found on: 192.168.0.22
Bulb found on: 192.168.0.24
Enter buld address: 192.168.0.24
Choose an action: (type "h" for list all)
 >

#second way
./main 192.168.2.244
Choose an action: (type "h" for list all)
 > 
```

After you "connect" to a bulb you can: 
 - Get system information like: 
   - Bulb MAC address.
   - Home ID.
   - Room ID.
   - Region.
   - Firmware version.
   - Etc.
 - Get bulbs status.
 - Power ON.
 - Power OFF.
 - Change color.
 - Change brightness.
 - Change scene.

### Compile
You can compile the program with: 
``` bash
make
```