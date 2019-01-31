#EDITING...
## Trouble Shoot: ssh_exchange_identification: read: Connection reset by peer

Every step you've made during the setup phase is clean and smooth. But you might run into a login issue at the end, which is "ssh_exchange identification: read: Connection reset by peer" (Image below)  
<table>
  <tr>
    <td> <img src = "images/1.png" width = "700"> </td>
  </tr>
</table>
Connecting to the wrong IP would be a reason. The IP leases do not start over from 192.168.15.4 when you create a new network for your virtual machine. This is because the VBox Manager DHCP server will continute assiging IP address in sequential ascending order. Then the next question will be "How can we find the address in the range from 192.168.154.4 to 192.168.154.255?"

<br />
Try  
<br />

> VBoxManage guestproperty get [VMNAME] "/VirtualBox/GuestInfo/Net/0/V4/IP"

to find your IP address of your virtual machine.

If you got a message: "No Value Set", chances are that your virtual machine never made it through boot. So your virtual machine has not done any DHCP requests/discovery, providing no IP number. Try to boot in display mode and see whether or not you get to the login prompt on your VM machine screen. If yes, then log in it, run Terimnal, and type "ifconfig" to find out the acutal IP address.

<br>
If your server doens't provide the virtualbox GUI, then here are some tools for troubleshooting.

1. VBoxManage controlvm [VMNAME] screenshotpng [IMGNAME] 
2. VBoxManage controlvm [VMNAME] keyboardputstring [STRING]
3. VBoxManage controlvm [VMNAME] keyboadputscancode [HEXCODE]

First of all, screenshot your virtual machine to see its current state.<br/>
> VBoxManage controlvm [VMNAME] screenshotpng img1.png
<table>
  <tr>
    <td> <img src = "images/2.png" width = "700"> </td>
  </tr>
</table>

- - - -

<br />
Open img1.png by using a graphical tool on your server; I downloaded the PNG file and opened it on local.
<table>
  <tr>
    <td> <img src = "images/img1.png" width = "700"> </td>
    <img1.png>
  </tr>
</table>

----
<br />

If the opened img1.png looks like the above image, then send your password to your virtual machine. Screenshot your virtual machine to see its state. (screenshot img2.png)

> VBoxManage controlvm [VMNAME] keyboardputstring [PASSWORD]

<table>
  <tr>
    <td> <img src = "images/img2.png" width = "700"> </td>
  </tr>
</table>

----
<br />

If your passwords in the box appears typed(check your img2.png), 
send "enter" to your virtual machine so you can actually log into your virtual machine. (screenshot img3.png)

> VBoxManage controlvm [VMNAME] keyboardputscancode 1c 9c

"1c 9c" is equivalent to "Press and Hold Enter(1c), and Relase it(9c)"

For any further information, check this website: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

<table>
  <tr>
    <td> <img src = "images/img3.png" width = "700"> </td>
  </tr>
</table>

----
<br />

If you're successfully logged in (check your img3.png), then open a search box(left alt + f2). The command will be

> VBoxManage controlvm [VMNAME] keyboardputscancode 38 3c b8 bc

Don't forget to screenshot your virtual machine's state as img4.png

<table>
  <tr>
    <td> <img src = "images/img4.png" width = "700"> </td>
  </tr>
</table>

----
<br />

The next thing will type "gnome-terminal" in the search box. 

Try

>  VBoxManage controlvm [VMNAME] keyboardputstring "gnome-terminal"

----
<br />

<table>
  <tr>
    <td> <img src = "images/img5.png" width = "700"> </td>
  </tr>
</table>

----
<br />

<table>
  <tr>
    <td> <img src = "images/img6.png" width = "700"> </td>
  </tr>
</table>

----
<br />

<table>
  <tr>
    <td> <img src = "images/img7.png" width = "700"> </td>
  </tr>
</table>

----
<br />

<table>
  <tr>
    <td> <img src = "images/img8.png" width = "700"> </td>
  </tr>
</table>

----
<br />

<table>
  <tr>
    <td> <img src = "images/3.png" width = "700"> </td>
  </tr>
</table>
