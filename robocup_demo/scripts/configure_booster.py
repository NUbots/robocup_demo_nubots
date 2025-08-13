#!/usr/bin/env python3
# WARNING: MAKE SURE THAT THE CWD IS ~/robocup_demo/scripts
"""
Interactive WiFi IP Configuration Override Tool
Forces static IP configuration on WiFi interfaces using NetworkManager
"""

import subprocess
import sys
import os
from dataclasses import dataclass
from typing import Optional


@dataclass
class WifiConfig:
    """Configuration struct for WiFi IP settings"""
    ssid: str
    interface: str
    ip: str
    subnet: str  # CIDR notation like "24" for /24
    gateway: str
    password: str = ""
    dns: str = "8.8.8.8"
    gc_ip: str = "127.0.0.1"


def run_command(cmd: list[str], check: bool = True) -> subprocess.CompletedProcess:
    """Run a shell command and return the result"""
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=check)
        return result
    except subprocess.CalledProcessError as e:
        print(f"Error running command {' '.join(cmd)}: {e}")
        return e


def check_interface_exists(interface: str) -> bool:
    """Check if the network interface exists"""
    result = run_command(["nmcli", "device", "show", interface], check=False)
    return result.returncode == 0


def get_available_interfaces() -> list[str]:
    """Get list of available WiFi interfaces"""
    result = run_command(["nmcli", "device"], check=False)
    if result.returncode != 0:
        return []
    
    interfaces = []
    for line in result.stdout.splitlines():
        if line.strip() and "wifi" in line.lower():
            parts = line.split()
            if len(parts) >= 1:
                # The interface name is the first column
                interface_name = parts[0]
                # Check if it's a valid interface name (not empty, not a header)
                if interface_name and not interface_name.startswith("DEVICE"):
                    interfaces.append(interface_name)
    
    return interfaces


def get_available_networks() -> list[str]:
    """Get list of available WiFi networks"""
    result = run_command(["nmcli", "device", "wifi", "list"], check=False)
    if result.returncode != 0:
        return []
    
    networks = []
    for line in result.stdout.splitlines():
        if line.strip() and not line.startswith("IN-USE"):
            parts = line.split()
            if len(parts) >= 2:
                ssid = parts[1]
                if ssid not in networks and ssid != "SSID":
                    networks.append(ssid)
    
    return networks


def disconnect_current(config: WifiConfig):
    """Disconnect from current WiFi network"""
    print(f"Disconnecting from current WiFi on {config.interface}...")
    run_command(["nmcli", "device", "disconnect", config.interface], check=False)


def create_connection(config: WifiConfig):
    """Create a new NetworkManager connection with static IP"""
    print(f"Creating connection to {config.ssid} with static IP {config.ip}/{config.subnet}...")
    
    # Remove existing connection if it exists
    run_command(["nmcli", "connection", "delete", config.ssid], check=False)
    
    # Create new connection with static IP
    cmd = [
        "nmcli", "connection", "add",
        "type", "wifi",
        "con-name", config.ssid,
        "ifname", config.interface,
        "ssid", config.ssid,
        "ip4", f"{config.ip}/{config.subnet}",
        "gw4", config.gateway,
        "ipv4.dns", config.dns,
        "ipv4.method", "manual"
    ]
    
    result = run_command(cmd)
    if result.returncode != 0:
        print("Failed to create connection")
        return False
    
    # Set password if provided
    if config.password:
        print("Setting WiFi password...")
        run_command([
            "nmcli", "connection", "modify", config.ssid,
            "wifi-sec.key-mgmt", "wpa-psk",
            "wifi-sec.psk", config.password
        ])
    
    return True


def activate_connection(config: WifiConfig):
    """Activate the NetworkManager connection"""
    print(f"Activating connection to {config.ssid}...")
    result = run_command(["nmcli", "connection", "up", config.ssid])
    return result.returncode == 0


def verify_connection(config: WifiConfig) -> bool:
    """Verify all configuration matches the config"""
    print("Verifying configuration...")
    all_good = True
    
    # Check if connection is active
    result = run_command(["nmcli", "connection", "show", "--active"])
    if config.ssid not in result.stdout:
        print(f"✗ Connection '{config.ssid}' is not active")
        all_good = False
    else:
        print(f"✓ Connection '{config.ssid}' is active")
    
    # Check IP address and subnet
    result = run_command(["ip", "addr", "show", config.interface])
    if result.returncode != 0:
        print(f"✗ Could not get IP information for {config.interface}")
        all_good = False
    else:
        # Parse the IP address lines properly
        expected_ip_subnet = f"{config.ip}/{config.subnet}"
        found_correct_ip = False
        
        for line in result.stdout.splitlines():
            if "inet " in line and expected_ip_subnet in line:
                found_correct_ip = True
                break
        
        if found_correct_ip:
            print(f"✓ IP {expected_ip_subnet} is configured on {config.interface}")
        else:
            print(f"✗ IP {expected_ip_subnet} is not configured on {config.interface}")
            print(f"Current IPs on {config.interface}:")
            for line in result.stdout.splitlines():
                if "inet " in line:
                    print(f"  {line.strip()}")
            all_good = False
    
    # Check gateway
    result = run_command(["ip", "route", "show", "default"])
    if config.gateway in result.stdout:
        print(f"✓ Default gateway {config.gateway} is configured")
    else:
        print(f"✗ Default gateway {config.gateway} is not configured")
        print("Current default routes:")
        for line in result.stdout.splitlines():
            if "default" in line:
                print(f"  {line.strip()}")
        all_good = False
    
    # Check DNS
    result = run_command(["cat", "/etc/resolv.conf"])
    if result.returncode == 0 and config.dns in result.stdout:
        print(f"✓ DNS server {config.dns} is configured")
    else:
        print(f"✗ DNS server {config.dns} is not configured")
        print("Current DNS servers:")
        for line in result.stdout.splitlines():
            if line.startswith("nameserver"):
                print(f"  {line.strip()}")
        all_good = False
    
    # Check SSID connection
    result = run_command(["nmcli", "device", "wifi", "list", "--rescan", "no"])
    if result.returncode == 0:
        connected_ssid = None
        for line in result.stdout.splitlines():
            if f" {config.interface} " in line and "*" in line:
                # Extract SSID from the line
                parts = line.split()
                if len(parts) >= 2:
                    connected_ssid = parts[1]
                    break
                
        
        if connected_ssid == config.ssid:
            print(f"✓ Connected to SSID '{config.ssid}'")
        else:
            print(f"✗ Not connected to SSID '{config.ssid}' (connected to: {connected_ssid})")
            all_good = False
    else:
        print("✗ Could not verify SSID connection")
        all_good = False
    
    return all_good


def display_config(config: WifiConfig):
    """Display current configuration"""
    print("\n" + "=" * 50)
    print("CURRENT WIFI CONFIGURATION")
    print("=" * 50)
    print(f"SSID:                 {config.ssid}")
    print(f"Interface:            {config.interface}")
    print(f"IP Address:           {config.ip}/{config.subnet}")
    print(f"Gateway:              {config.gateway}")
    print(f"DNS Server:           {config.dns}")
    print(f"Password:             {config.password if config.password else '(none)'}")
    print(f"GameController IP:    {config.gc_ip}")
    print("=" * 50)


def edit_config(config: WifiConfig) -> WifiConfig:
    """Interactive configuration editor"""
    print("\nConfiguration Editor")
    print("Press Enter to keep current value, or type new value")
    
    # # SSID
    # print(f"\nCurrent SSID: {config.ssid}")
    # new_ssid = input("New SSID (or Enter to keep): ").strip()
    # if new_ssid:
    #     config.ssid = new_ssid
    
    # # Interface
    # print(f"\nCurrent Interface: {config.interface}")
    # available_interfaces = get_available_interfaces()
    # if available_interfaces:
    #     print("Available interfaces:")
    #     for i, iface in enumerate(available_interfaces, 1):
    #         print(f"  {i}. {iface}")
    #     print("  0. Custom input")
        
    #     choice = input("Select interface (or Enter to keep current): ").strip()
    #     if choice == "0":
    #         new_interface = input("Enter interface name: ").strip()
    #         if new_interface:
    #             config.interface = new_interface
    #     elif choice.isdigit() and 1 <= int(choice) <= len(available_interfaces):
    #         config.interface = available_interfaces[int(choice) - 1]
    
    # IP Address
    print(f"\nCurrent IP: {config.ip}")
    new_ip = input("New IP address (or Enter to keep): ").strip()
    if new_ip:
        config.ip = new_ip
    
    # Subnet
    # print(f"\nCurrent Subnet: /{config.subnet}")
    # new_subnet = input("New subnet mask / (or Enter to keep): ").strip()
    # if new_subnet:
    #     config.subnet = new_subnet
    
    # Gateway
    print(f"\nCurrent Gateway: {config.gateway}")
    new_gateway = input("New gateway (or Enter to keep): ").strip()
    if new_gateway:
        config.gateway = new_gateway
    
    # DNS
    print(f"\nCurrent DNS: {config.dns}")
    new_dns = input("New DNS server (or Enter to keep): ").strip()
    if new_dns:
        config.dns = new_dns
    
    # Password
    print(f"\nCurrent Password: {config.password if config.password else '(none)'}")
    new_password = input("New password (or Enter to keep): ").strip()
    if new_password:
        config.password = new_password
    
    return config

def set_game_controller_ip(config: WifiConfig) -> WifiConfig:
    print(f"\nCurrent GameController IP: {config.gc_ip if config.gc_ip else '(none)'}")
    new_gc_ip = input("New GameController IP (or Enter to keep): ").strip()   
    if new_gc_ip:
        config.gc_ip = new_gc_ip
    return config

def main_menu():
    """Display main menu and handle user input"""
    while True:
        print("\n" + "=" * 50)
        print("BOOSTER WIFI IP OVERRIDE AND BRAIN CONFIGURATION TOOL")
        print("=" * 50)
        print("1. Show current configuration")
        print("2. Edit configuration")
        print("3. Scan for available networks")
        print("4. Scan for available interfaces")
        print("5. Apply configuration")
        print("6. Verify current connection")
        print("7. Test connection (ping gateway)")
        print("8. Reset to DHCP")
        print("9. Configure purpose")
        print("10. Configure game controller IP")
        print("0. Exit")
        print("=" * 50)
        
        choice = input("Select option: ").strip()
        
        if choice == "1":
            display_config(config)
        elif choice == "2":
            edit_config(config)
        elif choice == "3":
            scan_networks()
        elif choice == "4":
            scan_interfaces()
        elif choice == "5":
            apply_configuration(config)
        elif choice == "6":
            verify_connection(config)
        elif choice == "7":
            test_connection(config)
        elif choice == "8":
            reset_to_dhcp()
        elif choice == "9":
            configure_purpose()
        elif choice == "10":
            set_game_controller_ip(config)
        elif choice == "0":
            print("Goodbye!")
            sys.exit(0)
        else:
            print("Invalid option. Please try again.")

def configure_purpose():
    script_path = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(script_path, "../src/brain/config/config.yaml"), "r") as f:
        current_purpose = ""
        for line in f:
            if "player_role" in line:
                # Split by apostrophe and take the second part
                current_purpose = line.lstrip().split('"')[1]
                break
    
    print(f"Current purpose: {current_purpose}")
    new_purpose = input("Enter new purpose (striker or goal_keeper) or Enter to keep: ").strip()
    # Normalise the new purpose
    new_purpose = new_purpose.lower()
    if new_purpose not in ["striker", "goal_keeper"]:
        print("Invalid purpose. Please enter either striker or goal_keeper.")
        return
    
    lines = []
    if new_purpose:
        with open(os.path.join(script_path, "../src/brain/config/config.yaml"), "r") as f:
            lines = f.readlines()
        with open(os.path.join(script_path, "../src/brain/config/config.yaml"), "w") as f:
            for line in lines:
                if "player_role" in line:
                    line = f'      player_role: "{new_purpose}" # striker | goal_keeper\n'
                    print(f"New purpose set to: {new_purpose}")
                f.write(line)
    else:
        print("No new purpose provided")


def scan_networks():
    """Scan and display available WiFi networks"""
    print("\nScanning for available networks...")
    result = run_command(["nmcli", "device", "wifi", "rescan"])
    
    networks = get_available_networks()
    if networks:
        print(f"\nFound {len(networks)} networks:")
        for i, network in enumerate(networks, 1):
            print(f"  {i}. {network}")
    else:
        print("No networks found")


def scan_interfaces():
    """Display available network interfaces"""
    print("\nAvailable network interfaces:")
    result = run_command(["nmcli", "device"])
    if result.returncode == 0:
        print(result.stdout)
    else:
        print("Failed to get interface list")


def apply_configuration(config: WifiConfig):
    """Apply the WiFi configuration"""
    print("Applying team and player id configurations...\n")
    # The last 16 bits of the ip corresponds to team_id and player_id, so we edit the config here now before applying the wifi settings
    team_id, player_id = config.ip.split(".")[2:]

    # Write the team and player id to the brain config
    lines = []
    script_path = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(script_path, "../src/brain/config/config.yaml"), "r") as f:
        lines = f.readlines()

    with open(os.path.join(script_path, "../src/brain/config/config.yaml"), "w") as f:
        for line in lines:
            if "team_id" in line:
                line = f'      team_id: {team_id} # must be consistant with GameControl\n'
                print(f"New team id set: {team_id}")
            elif "player_id" in line:
                line = f'      player_id: {player_id} # 1 | 2 | 3 | 4 | 5\n'
                print(f'New player id set: {player_id}')
            elif "game_control_ip" in line:
                line = f'    game_control_ip: "{config.ip}"\n'
            f.write(line)

    print(f"\nApplying configuration for {config.ssid}...")
    
    # Check if running as root
    if subprocess.run(["id", "-u"], capture_output=True, text=True).stdout.strip() != "0":
        print("Error: This script must be run as root (use sudo)")
        print("Please run: sudo python3 override_wifi_interactive.py")
        return
    
    # Check if interface exists
    if not check_interface_exists(config.interface):
        print(f"Error: Interface {config.interface} does not exist")
        print("Available interfaces:")
        subprocess.run(["nmcli", "device"])
        return
    
    # Disconnect from current WiFi
    disconnect_current(config)
    
    # Create new connection with static IP
    if not create_connection(config):
        print("Failed to create connection")
        return
    
    # Activate the connection
    if not activate_connection(config):
        print("Failed to activate connection")
        return
    
    # Verify connection
    if not verify_connection(config):
        print("Configuration verification failed")
        return
    
    print("\nConfiguration completed successfully!")
    print(f"Interface {config.interface} is now configured with IP {config.ip}/{config.subnet}")
    print("The connection is persistent and will be restored on reboot.")


def test_connection(config: WifiConfig):
    """Test connection by pinging the gateway"""
    print(f"\nTesting connection to gateway {config.gateway}...")
    result = run_command(["ping", "-c", "3", "-W", "2", config.gateway], check=False)
    
    if result.returncode == 0:
        print("✓ Connection test successful!")
        # Extract ping statistics
        for line in result.stdout.splitlines():
            if "packets transmitted" in line:
                print(f"  {line.strip()}")
            elif "rtt min/avg/max" in line:
                print(f"  {line.strip()}")
    else:
        print("✗ Connection test failed")
        print("Check your configuration and try again")


def reset_to_dhcp():
    """Reset the current connection to use DHCP"""
    print("\nResetting to DHCP...")
    
    # Get current active WiFi connection using device status
    result = run_command(["nmcli", "device", "status"])
    if result.returncode != 0:
        print("Failed to get device status")
        return
    
    # Find the WiFi interface that's connected
    wifi_interface = None
    for line in result.stdout.splitlines():
        if "wifi" in line.lower() and "connected" in line.lower():
            parts = line.split()
            if len(parts) >= 1:
                wifi_interface = parts[0]
                break
    
    if not wifi_interface:
        print("No active WiFi connection found")
        return
    
    # Get the connection name for this interface
    result = run_command(["nmcli", "device", "show", wifi_interface])
    if result.returncode != 0:
        print(f"Failed to get device info for {wifi_interface}")
        return
    
    # Extract the connection name from device info
    active_connection = None
    for line in result.stdout.splitlines():
        if "GENERAL.CONNECTION:" in line:
            parts = line.split(":", 1)
            if len(parts) >= 2:
                active_connection = parts[1].strip()
                break
    
    if not active_connection:
        print(f"Could not find connection name for interface {wifi_interface}")
        return
    
    print(f"Resetting connection '{active_connection}' to DHCP...")
    
    # Modify connection to use DHCP
    result = run_command([
        "nmcli", "connection", "modify", active_connection,
        "ipv4.method", "auto"
    ])
    
    if result.returncode != 0:
        print(f"Failed to modify connection: {result.stderr}")
        return
    
    # Restart connection
    result = run_command(["nmcli", "connection", "down", active_connection])
    if result.returncode != 0:
        print(f"Failed to bring down connection: {result.stderr}")
        return
    
    result = run_command(["nmcli", "connection", "up", active_connection])
    if result.returncode != 0:
        print(f"Failed to bring up connection: {result.stderr}")
        return
    
    print("Connection reset to DHCP successfully")


if __name__ == "__main__":
    # Default configuration
    config = WifiConfig(
        ssid="5_FIELD_P",
        interface="wlp1s0",
        ip="192.168.57.1",
        subnet="16",
        gateway="192.168.9.1",
        password="12345678",
        dns="8.8.8.8",
        gc_ip="192.168.9.2"
    )
    
    # Check if running as root for operations that need it
    is_root = subprocess.run(["id", "-u"], capture_output=True, text=True).stdout.strip() == "0"
    
    if not is_root:
        print("Warning: Not running as root. Some operations may fail.")
        print("For full functionality, run: sudo python3 override_wifi_interactive.py")
        print()
    
    # Start interactive menu
    main_menu()