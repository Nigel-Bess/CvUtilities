#!/usr/bin/expect -f

set timeout -1
set ip [lindex $argv 0]
set pw FreshEngr
set yes "y"
spawn ./deploy.sh $ip $ip $yes

expect "password:"
send "$pw\r"
expect "password:"
send "$pw\r"
sleep 1
send eof