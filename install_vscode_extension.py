import subprocess

# List of required and optional extension IDs
required_extensions = [
    "platformio.platformio-ide",
    "ms-python.python",
    "ms-vscode.cpptools"
]

# Function to list installed extensions
def get_installed_extensions():
    result = subprocess.run(["code", "--list-extensions"], capture_output=True, text=True)
    return result.stdout.splitlines()

# Function to install extensions with confirmation
def install_extensions(extensions):
    installed_extensions = get_installed_extensions()
    for extension in extensions:
        if extension in installed_extensions:
            print(f"Skipping {extension}, already installed.")
        else:
            confirm = input(f"Install {extension}? (y/n): ")
            if confirm.lower() == 'y':
                print(f"Installing {extension}...")
                subprocess.run(["code", "--install-extension", extension], check=True)
            else:
                print(f"Skipping {extension}.")

if __name__ == "__main__":
    try:
        # Install required extensions with confirmation
        install_extensions(required_extensions)
        
        print("Installation process complete.")
    except subprocess.CalledProcessError as e:
        print(f"An error occurred while installing extensions: {e}")
