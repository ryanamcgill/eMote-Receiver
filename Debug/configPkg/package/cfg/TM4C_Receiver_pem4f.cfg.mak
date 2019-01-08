# invoke SourceDir generated makefile for TM4C_Receiver.pem4f
TM4C_Receiver.pem4f: .libraries,TM4C_Receiver.pem4f
.libraries,TM4C_Receiver.pem4f: package/cfg/TM4C_Receiver_pem4f.xdl
	$(MAKE) -f C:\Users\ram0063\Desktop\STORML~1\CCS_Workspace\Receiver/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\ram0063\Desktop\STORML~1\CCS_Workspace\Receiver/src/makefile.libs clean

