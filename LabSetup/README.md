#EDITING...
## Trouble Shoot: ssh_exchange_identification: read: Connection reset by peer

Every step you've made during the setup phase is clean and smooth. But you might run into a login issue at the end, which is "ssh_exchange identification: read: Connection reset by peer" (Image below)

<table><tr>
  <td> <img src = "images/1.png" width = "700"> </td>
  </tr>
</table>

Connecting to the wrong IP would be a reason. The IP leases do not start over from 192.168.15.4 when you create a new network for your virtual machine. That is because the VBox Manager DHCP server will continute assiging IP address in sequential ascending order. 

Try
  VBoxManage guestproperty get [VMNAME] "/VirtualBox/GuestInfo/Net/0/V4/IP" 
to find your IP address of your virtual machine.

If it didn't work saying "No Value Set", then 


<table><tr>connecting
  <td> <img src = "images/2.png" width = "700"> </td>
  </tr>
</table>

<table><tr>
  <td> <img src = "images/2.png" width = "700"> </td>
  </tr>
</table>

<table><tr>
  <td> <img src = "images/2.png" width = "700"> </td>
  </tr>
</table>

<table><tr>
  <td> <img src = "images/3.png" width = "700"> </td>
  </tr>
</table>


