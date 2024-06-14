#include "blink.h"

void BlinkCleanup(ClientData data) {
  BlinkState *statePtr = (BlinkState *)data;
  Blinker *blinkPtr;
  Tcl_HashEntry *entryPtr;
  Tcl_HashSearch search;

  entryPtr = Tcl_FirstHashEntry(&statePtr->hash, &search);
  while (entryPtr != NULL) {
    blinkPtr = Tcl_GetHashValue(entryPtr);
    BlinkDelete(blinkPtr, entryPtr);
    entryPtr = Tcl_FirstHashEntry(&statePtr->hash, &search);
  }

  Tcl_Free((char *)statePtr);
}

/// Return the number of attached Blink(1) devices.
static int BlinkEnumerate(Tcl_Interp *interp) {
  Tcl_Obj *objPtr = Tcl_NewIntObj(blink1_enumerate());
  Tcl_SetObjResult(interp, objPtr);
  
  return TCL_OK;
}

/// Return a list whose entries describe each attached Blink(1).
static int BlinkList(Tcl_Interp *interp) {
  Tcl_Obj *const objects[0];
  Tcl_Obj *listPtr = Tcl_NewListObj(0, objects);

  Tcl_Obj *devIDKeyPtr = Tcl_NewStringObj("Device ID", -1);
  Tcl_Obj *serialKeyPtr = Tcl_NewStringObj("Serial", -1);
  Tcl_Obj *typeKeyPtr = Tcl_NewStringObj("Type", -1);
  
  int n = blink1_enumerate();
  for (int i = 0; i < n; i++) {
    Tcl_Obj *dict = Tcl_NewDictObj();
    Tcl_DictObjPut(interp, dict, devIDKeyPtr, Tcl_NewIntObj(i));
    Tcl_DictObjPut(interp, dict, serialKeyPtr, Tcl_NewStringObj(blink1_getCachedSerial(i), -1));
    Tcl_DictObjPut(interp, dict, typeKeyPtr, Tcl_NewIntObj(blink1_deviceTypeById(i)));

    if (Tcl_ListObjAppendElement(interp, listPtr, dict) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, listPtr);
  return TCL_OK;
}

static int inrange(int color) {
  return 0 <= color && color <= 255;
}

/// Set the specified Blink(1) to display the specified color.
static int BlinkSetRGB(Tcl_Interp *interp, Blinker *blinkPtr, int red, int green, int blue) {
  if (blinkPtr == NULL) {
    // TODO: can this even occur?
    // TODO: some sort of error message
    return TCL_ERROR;
  }

  if (blinkPtr->device == NULL) {
    // TODO: possible? or did we already check?
    // TODO: return an error message
    return TCL_ERROR;
  }

  if (!inrange(red)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("red value out of range", -1));
    return TCL_ERROR;
  }
    
  if (!inrange(green)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("green value out of range", -1));
    return TCL_ERROR;
  }
    
  if (!inrange(blue)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("blue value out of range", -1));
    return TCL_ERROR;
  }
    
  int result = blink1_setRGB(blinkPtr->device, red, green, blue);
  // TODO: check result

  return TCL_OK;
}

#define SET(Color, r, g, b) \
  static int Blink##Color(Tcl_Interp *interp, Blinker *blinkPtr) {        \
    return BlinkSetRGB(interp, blinkPtr, r, g, b);                       \
  }

/// Set the specified Blink(1) to display white.
SET(On, 255, 255, 255)

/// Turns the specified Blink(1) off.
SET(Off, 0, 0, 0)

/// Turns the specified Blink(1) off.
SET(Black, 0, 0, 0)

/// Set the specified Blink(1) to display white.
SET(White, 255, 255, 255)

/// Set the specified Blink(1) to display red.
SET(Red, 255, 0, 0)

/// Set the specified Blink(1) to display green.
SET(Green, 0, 255, 0)

/// Set the specified Blink(1) to display blue.
SET(Blue, 0, 0, 255)

/// Set the specified Blink(1) to display cyan.
SET(Cyan, 0, 255, 255)

/// Set the specified Blink(1) to display magenta.
SET(Magenta, 255, 0, 255)

/// Set the specified Blink(1) to display yellow.
SET(Yellow, 255, 255, 0)

/// Set the specified Blink(1) to display orange.
SET(Orange, 255, 165, 0)

/// Given a device ID, "open"s the corresponding device.
static int BlinkOpen(Tcl_Interp *interp, BlinkState *statePtr, Tcl_Obj *objPtr) {
  Tcl_HashEntry *entryPtr;
  int new;
  Blinker *blinkPtr;
  char name[20];
  int devid;

  // TODO: if no devid specified, open the first device found.
  
  // TODO: for now we're only implementing opening by device ID
  // we'll come back and do serial # later

  if (objPtr != NULL) {
    if (Tcl_GetIntFromObj(interp, objPtr, &devid) != TCL_OK) {
      return TCL_ERROR;
    }
  } else {
    // shouldn't be possible
    return TCL_ERROR;
  }

  int n = blink1_enumerate(); // need to enumerate before trying to create a device

  blinkPtr = Tcl_Alloc(sizeof(Blinker));
  blinkPtr->device = NULL;
  blinkPtr->device = blink1_openById(devid);

  if (blinkPtr->device == NULL) {
    // signal error
    //printf("Got an error opening device\n");

    return TCL_ERROR; // TCL_ERROR or a blink-specific error?
  }

  sprintf(name, "blink%d", devid);
  entryPtr = Tcl_CreateHashEntry(&statePtr->hash, name, &new);
  Tcl_SetHashValue(entryPtr, (ClientData)blinkPtr);

  Tcl_SetStringObj(Tcl_GetObjResult(interp), name, -1);
  // Tcl_Obj *resultPtr = Tcl_NewStringObj(name, -1);
  // Tcl_SetObjResult(interp, resultPtr);

  return TCL_OK;
}

/// Turns the specified Blink(1) off and detaches it.
static int BlinkClose(Blinker *blinkPtr) {
  if (blinkPtr->device != NULL) {
    blink1_setRGB(blinkPtr->device, 0, 0, 0);
    blink1_close(blinkPtr->device);
    blinkPtr->device = NULL;
  }

  return TCL_OK;
}

int BlinkDelete(Blinker *blinkPtr, Tcl_HashEntry *entryPtr) {
  Tcl_DeleteHashEntry(entryPtr);

  if (blinkPtr->device != NULL) {
    // close the blink device ...
  }

  Tcl_EventuallyFree((char *)blinkPtr, Tcl_Free);
  return TCL_OK;
}

/// Returns ThingM's vendor ID.
static int BlinkVid(Tcl_Interp *interp) {
  Tcl_Obj *objPtr = Tcl_NewIntObj(blink1_vid());
  Tcl_SetObjResult(interp, objPtr);

  return TCL_OK;
}

/// Returns the Blink(1) product ID.
static int BlinkPid(Tcl_Interp *interp) {
  Tcl_Obj *objPtr = Tcl_NewIntObj(blink1_pid());
  Tcl_SetObjResult(interp, objPtr);

  return TCL_OK;
}

// TODO: would this be simpler as an ensemble command?
int BlinkCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  BlinkState *statePtr = (BlinkState *)data;
  Tcl_Obj *valueObjPtr = NULL;

  char *subCmds[] = {
    "black", "blue", "close", "cyan", "enumerate", "green",
    "list", "magenta", "off", "on", "open", "orange", "pid",
    "red", "set", "vid", "white", "yellow",
    NULL
  };

  int numArgs[] = {
    3, 3, 3, 3, 2, 3,
    2, 3, 3, 3, 3, 3, 2,
    3, 6, 2, 3, 3,
  };
  
  enum BlinkIx {
    BlackIx, BlueIx, CloseIx, CyanIx, EnumerateIx, GreenIx,
    ListIx, MagentaIx, OffIx, OnIx, OpenIx, OrangeIx, PidIx,
    RedIx, SetIx, VidIx, WhiteIx, YellowIx,
  };
  int result, index;
 
  if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "option", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }
  
  if (objc != numArgs[index]) {
    // TODO: message in following call should be "describe all the options after blink ..."
    Tcl_WrongNumArgs(interp, 1, objv, "option ... TBD ...");
    return TCL_ERROR;
  }

  switch(index) {
  case EnumerateIx:
    return BlinkEnumerate(interp);
  case ListIx:
    return BlinkList(interp);
  case OpenIx:
    return BlinkOpen(interp, statePtr, objv[2]);
  case VidIx:
    return BlinkVid(interp);
  case PidIx:
    return BlinkPid(interp);
  }

  // The following commands take a blink name as the third parameter:

  Tcl_HashEntry *entryPtr = Tcl_FindHashEntry(&statePtr->hash, Tcl_GetString(objv[2]));
  if (entryPtr == NULL) {
    Tcl_AppendResult(interp, "unknown blink: ", Tcl_GetString(objv[2]), NULL);
    return TCL_ERROR;
  }
  Blinker *blinkPtr = (Blinker *)Tcl_GetHashValue(entryPtr);
  
  switch(index) {
  case CloseIx:
    return BlinkClose(blinkPtr);
  case OnIx:
    return BlinkOn(interp, blinkPtr);
  case OffIx:
    return BlinkOff(interp, blinkPtr);
  case BlackIx:
    return BlinkBlack(interp, blinkPtr);
  case WhiteIx:
    return BlinkWhite(interp, blinkPtr);
  case RedIx:
    return BlinkRed(interp, blinkPtr);
  case GreenIx:
    return BlinkGreen(interp, blinkPtr);
  case BlueIx:
    return BlinkBlue(interp, blinkPtr);
  case CyanIx:
    return BlinkCyan(interp, blinkPtr);
  case MagentaIx:
    return BlinkMagenta(interp, blinkPtr);
  case YellowIx:
    return BlinkYellow(interp, blinkPtr);
  case OrangeIx:
    return BlinkOrange(interp, blinkPtr);
  case SetIx:
    {
      int r, g, b;
      Tcl_GetIntFromObj(interp, objv[3], &r);
      Tcl_GetIntFromObj(interp, objv[4], &g);
      Tcl_GetIntFromObj(interp, objv[5], &b);
      return BlinkSetRGB(interp, blinkPtr, r, g, b);
    }
  }

  // TODO: shouldn't really allow dropping through ... is it even possible?
  return TCL_OK;
}

// Library entry point.
int Blink_Init(Tcl_Interp *interp) {
  BlinkState *statePtr;

  // TODO: do we need stubs? TBH I don't really understand what they are.
  // Need to find and read the relevent documentation.
  if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
    return TCL_ERROR;
  }
  
  statePtr = (BlinkState *)Tcl_Alloc(sizeof(BlinkState));
  Tcl_InitHashTable(&statePtr->hash, TCL_STRING_KEYS);
  statePtr->uid = 0; // TODO: probably don't need this; left over from example
  
  Tcl_CreateObjCommand(interp, "blink", BlinkCmd, (ClientData)statePtr, BlinkCleanup);
  Tcl_PkgProvide(interp, "blink", "1.0");

  return TCL_OK;
}
