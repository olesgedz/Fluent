# Build

## Install dependencies

#### On moment of writing this build guide actual vulkan sdk version 1.2.189 for unix systems and 1.2.189.2 for windows

## Linux

### Arch

```shell
# We will need xorg
sudo pacman -S xorg
# I will use amd vulkan implementation
yay -S vulkan-amdgpu-pro
# Install base vulkan packages
sudo pacman -S vulkan-devel
# Also we need aditional libs
sudo pacman -S shaderc
yay -S spirv-cross
```
### Ubuntu
```shell
# Install xorg
sudo apt install xorg-dev
# We just need to install sdk
wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.189-focal.list https://packages.lunarg.com/vulkan/1.2.189/lunarg-vulkan-1.2.189-focal.list
sudo apt update
sudo apt install vulkan-sdk
```

## Windows

#### You can also download it from lunarg website but I will show only terminal version

```shell
# Download installer
Invoke-WebRequest "https://sdk.lunarg.com/sdk/download/1.2.189.2/windows/VulkanSDK-1.2.189.2-Installer.exe" -OutFile VulkanSDK.exe -v
# Launch installer (visual mode, don't forget debug libs)
.\VulkanSDK.exe
# Or continue in terminal (You will need manually setup enviroment later)
.\VulkanSDK.exe --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.debug
```

#### After installation you should reboot pc or re-login in account

## MacOS (Not supported now, can build it but it will crashes on launch)

### Similar to Windows you can download it directly from website

```shell
# Download installer
curl https://sdk.lunarg.com/sdk/download/1.2.189.0/mac/vulkansdk-macos-1.2.189.0.dmg --output VulkanSDK.dmg
# You can launch graphical installation now
# Or continue in terminal
# Attach dmg 
hdiutil attach ./VulkanSDK.dmg
sudo /Volumes/vulkansdk-macos-1.2.189.0/InstallVulkan.app/Contents/MacOS/InstallVulkan --accept-licenses --default-answer --confirm-command install
```