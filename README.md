# yubicrack-improved
An improved Yubikey cracker incorporating fixes from other repos

Works on MacOS and Linux, possibly windows too


# How to use

Download yubico-c and build/install using their instructions:
https://developers.yubico.com/yubico-c-client/

Now use 'make' to build



run with:
./yubicrack
./yubicrack -y # -y to skip start message
./yubicrack -y 000000000000 # replace zeros with place to start/resume (printed every 50 attempts)


# Tip for success
Cracking sucks, its slow and hard but since the yubikey library isnt optomized for cracking you can dramatically increase speed by running the binary multiple times with the resume set to differnt values.
example:
Terminal 0 ./yubicrack -y 000000000000
Terminal 1 ./yubicrack -y 100000000000
Terminal 2 ./yubicrack -y 200000000000
Terminal 3 ./yubicrack -y 300000000000

I seem to be able to run about 15 sessions at a time on a macbook

In the future i may wright a script to manage how many instances and increase until it impacts the speed of others to get the most out of the USB
