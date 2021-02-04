# nose (Graduate Design)
A simple VPN implement over p2p



## Project structure

```

                                              +--------------+       +---------------+       +--------------+
                                       +----->| local client |<----->| public server |<----->| local client |<-----+
                                       |      +--------------+       +---------------+       +--------------+      |
                                       |                                                                           |
 +---------+        +-------------+    |                                                                           |    +-------------+       +--------+
 |  peer A | <----->| utun device |<---+ |<---------------------------- VPN tunnel ----------------------------->| +--->| utun device |<----->| peer B |
 +---------+        +-------------+                                                                                     +-------------+       +--------+
```

## Project requirements
This project built depends on some third party open sources.

Dependencies list:
```
libuv 1.31.1-dev
openssl 1.1.1d
mbedtls 2.25.0
```

## Command line usage

```
nose is a very simple VPN implement over p2p

Usage: nose [server|client] 
  -l IP address assigned to the utun interface of local machine
  -r IP address assigned to the other peer of p2p
  -sh IP address of public server for forwarding the traffic
  -sp Port number given to the public server


Examples:
In client:
nose client -l 10.1.0.10 -r 10.1.0.20 -sh x.x.x.x -sp 9090
In server:
nose server -sh x.x.x.x -sp 9090

```
**Note**:

For the server, firstly make sure your firewall software allow to access to the port you assign to the program, or it will make the connection failed. Addtionally, although even you shutdown the firewall, the problem would still occur partly because your host provider builds the firewall in the traffic entrance of the server. To solve this, login to your console panel and add the accessible rules accordingly. 

For the client, the program should be run as superuser or other users which are equal to the privilege of superuser. 

## VPN protocol(draft)
###  Preface
Comparing with serveral famous VPN protocol——PPP, PPTP, SSL VPN, etc. and learning their natures and features, I finally present a relatively simple and available VPN protocol(Under daft) conbined with respectively advantages, including the **Authentiction** and **Encryption** which both meet the almost application scene for safety and protection.

### Authentication
It references from CHAP, Challenge-Handshake Authentication Protocol, which is applied in Point-to-Point Protocol(PPP). And I omit the previous Link Control Protocol for exchanging the id bewteen two sides, and use the random number to generate the id and challenge. Like CHAP, the whole process vertifies the identity of the client by using a there-way handshake. The vertifiction is based on a shared secret previously negotiated.

#### Work cycle

### Encryption 
### DFA
The VPN's deterministic finite automation is described by a five-element tuple: ![](https://latex.codecogs.com/gif.latex?\(Q,%20\sum,%20%20\delta,%20q_0,%20F\)), it can be represented by diagrams of this form:

![vpn_dfa.png](./imgs/vpn_dfa.png)


Defined the states as follow:

![](https://latex.codecogs.com/gif.latex?s_0=Start,%20s_1=Authentication,%20s_2=Key\%20exchange,%20s_3=Running)


Defined the input or symbols as follow:

![](https://latex.codecogs.com/gif.latex?c_0=Start\%20authenicating,%20c_1=Authentication\%20failed\%20or\%20timeout,%20c_2=Start\%20key\%20exchange,%20c_3=Key\%20exchange\%20failed\%20or\%20timeout,%20c_4=Key\%20exchange\%20success,%20c_5=Send\%20the\%20payload)

![](https://latex.codecogs.com/gif.latex?Q=\{%20s_0,%20s_1,%20s_2,%20s_3\}) 

![](https://latex.codecogs.com/gif.latex?\sum%20=%20\{c_0,%20c_1,%20c_2,%20c_3,%20c_4,%20c_5\})

The following table describles ![](https://latex.codecogs.com/gif.latex?\delta):

  | current state | input symbol | new state|
  | ---- | ------- |  ----- | 
  | ![](https://latex.codecogs.com/gif.latex?s_0) | ![](https://latex.codecogs.com/gif.latex?c_0) |  ![](https://latex.codecogs.com/gif.latex?s_1) |
  | ![](https://latex.codecogs.com/gif.latex?s_1) | ![](https://latex.codecogs.com/gif.latex?c_1) | ![](https://latex.codecogs.com/gif.latex?s_0)|
  | ![](https://latex.codecogs.com/gif.latex?s_1) | ![](https://latex.codecogs.com/gif.latex?c_2) | ![](https://latex.codecogs.com/gif.latex?s_2)|
  | ![](https://latex.codecogs.com/gif.latex?s_2)  | ![](https://latex.codecogs.com/gif.latex?c_3) | ![](https://latex.codecogs.com/gif.latex?s_1) |
  | ![](https://latex.codecogs.com/gif.latex?s_2)  | ![](https://latex.codecogs.com/gif.latex?c_4)|  ![](https://latex.codecogs.com/gif.latex?s_3)|
  | ![](https://latex.codecogs.com/gif.latex?s_3)  | ![](https://latex.codecogs.com/gif.latex?c_5) | ![](https://latex.codecogs.com/gif.latex?s_3)|

![](https://latex.codecogs.com/gif.latex?q_0=s_0)

![](https://latex.codecogs.com/gif.latex?F=s_0)


## Tasklist

### Must do
- [x] Optimize the method of opening utun device   

- [x] Data traffic bewteen utun device and local client
- [x] Utun implement in Linux platform
- [ ] Implement a relatively safe and simple VPN

### May do

- [ ] Utun implement in Windows platform