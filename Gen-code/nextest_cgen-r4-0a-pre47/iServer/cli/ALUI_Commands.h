#ifndef ALUI_COMMANDS_H
#define ALUI_COMMANDS_H

int
ALUI_SetNetoidList( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );
int
ALUI_PopulateNetoidList( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );
int
ALUI_ProfileNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );

int
ALUI_DetailNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );
int
ALUI_DeleteNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );
int
ALUI_AddNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );
int
ALUI_FindNetoid( ClientData clientdata, Tcl_Interp *interp,
               int argc, char *argv[] );

extern Tcl_Interp *gInterp;

#endif
