#!/bin/bash

# Check if 'expect' is installed
if ! command -v expect &> /dev/null; then
	echo "'expect' is not installed on local machine. Installing it now..."
	# Update package lists and install 'expect'
	sudo apt-get update
	sudo apt-get install -y expect

	# Check if installation was successful
	if ! command -v expect &> /dev/null; then
		echo "Failed to install 'expect' on local machine. Please check for errors."
		exit 1
	fi
else
	echo "'expect' is installed on local machine."
fi
