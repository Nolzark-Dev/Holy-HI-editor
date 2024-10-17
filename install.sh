#!/bin/bash


echo "        ___                    "
echo "      /\\  \\                   "
echo "      \\:\\  \\      ___         "
echo "       \\:\\  \\    /\\__\\        "
echo "   ___ /::\\  \\  /:/__/        "
echo "  /\\  /:/\\:\\__\\/::\\  \\        "
echo "  \\:\\/:/  \\/__/\\/\\:\\  \\__     "
echo "   \\::/__/      ~~\\:\\/\\__\\    "
echo "    \\:\\  \\         \\::/  /    "
echo "     \\:\\__\\        /:/  /     "
echo "      \\/__/        \\/__/      "
echo "                               "
echo "Welcome to the HI TE Installer"
echo "=============================="
echo "Starting the compilation and installation process..."




read -p "Do you want to install the program? (y/n): " answer

# Convert input to lowercase for easy matching
answer=$(echo "$answer" | tr '[:upper:]' '[:lower:]')

# If the user agrees to proceed
if [[ "$answer" == "y" || "$answer" == "yes" ]]; then
  echo "Starting the compilation and installation process..."







gcc -o hi hi.c -lncurses

if [ $? -ne 0 ]; then
  echo "Compilation failed. Please check your source code or troubleshooting guide"
  exit 1
fi

echo "Compilation successful."

sudo mv hi /usr/local/bin/
if [ $? -ne 0 ]; then
  echo "Failed to move the binary to /usr/local/bin/. Try running the script with sudo."
  exit 1
fi

echo "hi has been successfully installed in /usr/local/bin/"

echo "Installation complete. You can now run the program using 'hi'."

