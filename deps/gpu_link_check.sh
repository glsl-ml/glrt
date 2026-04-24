#!/bin/bash

# Ensure the script is run with sudo
if [ "$EUID" -ne 0 ]; then 
  echo "Please run as root (sudo)"
  exit
fi

# Header
printf "%-12s \t %-12s \t %-s\n" "PCI ID" "WIDTH" "DEVICE NAME"
echo "------------------------------------------------------------------------------------------------"

# Find all VGA and 3D controllers
lspci -nn | grep -E "VGA|3D" | while read -r line; do
    # Extract the PCI ID (e.g., 01:00.0)
    pci_id=$(echo "$line" | cut -d' ' -f1)
    
    # Clean the name: 
    # 1. Remove the bracketed class info at the start
    # 2. Remove the hexadecimal IDs at the end [1002:675d]
    full_name=$(echo "$line" | sed -e 's/^.*]: //' -e 's/ \[....:....\]//')

    # Query lspci for the Link Status (LnkSta)
    link_width=$(lspci -vv -s "$pci_id" 2>/dev/null | grep "LnkSta:" | grep -o "Width x[0-9]*" | cut -d' ' -f2)

    # Print results
    printf "%-12s \t %-12s \t %-s\n" "$pci_id" "$link_width" "$full_name"
done
