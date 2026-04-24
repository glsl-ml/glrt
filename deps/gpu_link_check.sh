#!/bin/bash

# Ensure the script is run with sudo
if [ "$EUID" -ne 0 ]; then 
  echo "Please run as root (sudo)"
  exit
fi

# Header
printf "%-12s \t %-8s \t %-10s \t %-s\n" "PCI ID" "WIDTH" "SPEED" "DEVICE NAME"
echo "------------------------------------------------------------------------------------------------"

# Find all VGA and 3D controllers
lspci -nn | grep -E "VGA|3D" | while read -r line; do
    # Extract the PCI ID
    pci_id=$(echo "$line" | cut -d' ' -f1)
    
    # Clean the name: Remove class info [0300] and hex IDs [1002:675d]
    full_name=$(echo "$line" | sed -e 's/^.*]: //' -e 's/ \[....:....\]//')

    # Query lspci for Link Status (LnkSta)
    lnk_sta=$(lspci -vv -s "$pci_id" 2>/dev/null | grep "LnkSta:")

    # Extract Width (e.g., x16)
    link_width=$(echo "$lnk_sta" | grep -o "Width x[0-9]*" | cut -d' ' -f2)

    # Extract Speed (e.g., 5GT/s or 8GT/s)
    link_speed=$(echo "$lnk_sta" | grep -o "[0-9.]*GT/s")

    # Print results
    printf "%-12s \t %-8s \t %-10s \t %-s\n" "$pci_id" "$link_width" "$link_speed" "$full_name"
done
