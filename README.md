# Waajacu oauth_server
This server ilustrates how to build a lightweight C server that authenticate users using OAuth.

# Requires 
- libmicrohttpd
- lgnutls
- lcurl
# Implementation instructions
Access the webpage https://oauth.net/playground/client-registration.html

Follow the instructions to register your application and a client

You will obtain the following: 

Client Registration
```
client_id 	example_id
client_secret 	example_secret
```
User Account
```
login 	login@example.com
password 	Beautiful-password-00
```
Remmeber the user login email and password.

For client_id and client_secret, use them to fill the contents of this files:
```
./access/oauth_client_id.key
./access/oauth_client_secret.key
```

Client registration
client_id 	7CCUISrBbfkLdJ46p7RohFKo
client_secret 	P4cALOd0EvqXlYuFvZhtcbKyZYmDF95WynCAb0Eelogp3Rx6
User Account
login 	misty-octopus@example.com
password 	Elated-Dormouse-99



# Build instructions
download latest version of libcurl https://curl.se/download.html 
```
[user@waajacu ~]$ cd ./path/to/Downloads
[user@waajacu Downloads]$ .tar zxvf curl.*.tar.gz
[user@waajacu Downloads]$ cd ./curl.*
```
download latest version of libmicrohttpd https://www.gnu.org/software/libmicrohttpd/
```
[user@waajacu ~]$ cd ./path/to/Downloads
[user@waajacu Downloads]$ .tar zxvf libmicrohttpd.*.tar.gz
[user@waajacu Downloads]$ cd ./libmicrohttpd.*
```
Install libmicrohttpd
```
[user@waajacu libmicrohttpd]$ ./configure --with-gcrypt=/usr/lib64/ --with-gnutls=/usr/lib64/
[user@waajacu libmicrohttpd]$ make
[user@waajacu libmicrohttpd]$ sudo make install
```
Install dynamic dependencies
```
[user@waajacu ~]$ glibc++
[user@waajacu ~]$ libstdc++
```
Install static dependencies (optional, required to compile static)
```
[user@waajacu ~]$ gnutls-static
[user@waajacu ~]$ glibc++-static
[user@waajacu ~]$ libstdc++-static
```
Install liboauth
```
[user@waajacu ~]$ dnf install liboauth-devel
```