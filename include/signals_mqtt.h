void signals_config_lamps(JsonObject& parsed);


/*
Takes a JsonObject and sets the environment
Reads mode and n_slaves from the JsonObject
This needs to be called before setSlave()
*/
void setEnvironment(ArduinoJson::JsonObject &parsed);

/*
Takes a JsonObject and sets the slave instance
Sets the opposite slave ID
Sets the commanded states for primary and secondary
Sets the timer values for primary and secondary
Format:
{
    "n":    ,
    "mode": ,
    "slaves": {
        "1" : {
            "state":    ,
            "red":      ,
            "amber":    ,
            "green":    
        },
        "2" : {
            "state":    ,
            "red":      ,
            "amber":    ,
            "green":    
        },
        ...
    }
}
*/
void setSlave(ArduinoJson::JsonObject &parsed);

/*
Returns the timer number of the specific lamp with lampID in LampID enum
*/
int getTimerNum(int lampID);

/*
Returns the elapsed time for the specific lamp
*/
unsigned int getElapsedTime(int lampID);

/*
Returns the red, green, amber timer values
lampID in LampID
colour in LampState
*/
int getTimerValues(int lampID, int colourID);

/*
Returns the remaining time for the specific lamp
*/
unsigned int getRemainingTime(int lampID);

/*
Returns the state of the primary lamp
*/
int getPrimaryState();

/*
Returns the state of the secondary lamp
*/
int getSecondaryState();

/*
Returns the state of the overhead lamp
*/
int getOverheadState();

/*
Returns the state of the spare lamp
*/
int getSpareState();