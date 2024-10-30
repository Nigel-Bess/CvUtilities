#!/usr/bin/expect -f

# use expect to run the script
set timeout -1
set ip [lindex $argv 0]
set pw FreshEngr
set yes "y"
spawn /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/deploy.sh $ip $ip $yes

expect {
"password:" { send "$pw\r"; exp_continue }
"yes/no" { send "yes\r"; exp_continue }
eof { exit 0 }
}
