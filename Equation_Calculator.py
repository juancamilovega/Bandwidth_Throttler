#Python script to do the equations described.

def Ceil(A):
    return -((-A)//1.0)

def Get_Penalty(BW,PS,HS,DW,SF,CR):
    BW = BW.lower()
    if (BW.endswith("gbps")):
        try:
            BW=float(BW.split("gbps")[0])*1000
            pen = 1.0*CR*SF*(PS + HS)/BW/Ceil(PS/DW)- SF
        except:
            print("Not a valid input")
            BW = -1
            pen = -1
    elif (BW.endswith("mbps")):
        try:
            BW=float(BW.split("mbps")[0])
            pen = 1.0*CR*SF*(PS + HS)/BW/Ceil(PS/DW)- SF
        except:
            BW = -1
            pen = -1
    elif (BW.endswith("kbps")):
        try:
            BW=float(BW.split("kbps")[0])/1000.0
            pen = 1.0*CR*SF*(PS + HS)/BW/Ceil(PS/DW)- SF
        except:
            BW = -1
            pen = -1
    else:
        try:
            BW=float(BW)/1000000.0
            pen = 1.0*CR*SF*(PS + HS)/BW/Ceil(PS/DW)- SF
        except:
            BW = -1
            pen = -1
    print("Penalty is :"+str(pen)+"Rounded to "+str(Ceil(pen)))
    print("For a bandwidth of "+str(Get_BWE(Ceil(pen),PS,HS,DW,SF,CR)))
    return(Ceil(pen))

def Get_BWE(Penalty,PS,HS,DW,SF,CR):
    if (Penalty<0):
        return -1
    return (1.0*CR*SF*(PS + HS)/(Penalty + SF)/Ceil(PS/DW))
