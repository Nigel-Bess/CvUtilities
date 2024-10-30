#!/usr/bin/expect -f

set timeout -1
set ip [lindex $argv 0]
set pw FreshEngr
set local_version_output [lindex $argv 1]

# SSH into the machine with the given IP
spawn /bin/bash /home/fulfil/code/Fulfil.ComputerVision/scripts/get_and_compare_version.sh $ip $local_version_output 

expect {
"password:" { send "$pw\r"; exp_continue }
"yes/no" { send "yes\r"; exp_continue }
eof
}

# capture the output of get_and_compare_version.sh
set output $expect_out(buffer)

# Extract the final hostname and version output line
set output_lines [split $output "\n"]
set final_output [lindex $output_lines end]
puts $final_output

