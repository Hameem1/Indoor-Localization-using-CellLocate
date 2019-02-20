//Adding a \n at the end of every printf statement is necessary to clear the buffer

#include "mbed.h"
#include <string>
#include "UbloxATCellularInterfaceExt.h"
#include "gnss.h"

#define PIN         "1171"
#define APN         "web.vodafone.de"
#define USERNAME    NULL
#define PASSWORD    NULL

// #define PIN         "0545"
// #define APN         "internet.t-mobile"      //(or) internet.telekom
// #define USERNAME    "t-mobile"       //(or) congstar
// #define PASSWORD    "tm"     //(or) cs

void shutdown_sequence(UbloxATCellularInterfaceExt *,GnssSerial);
static string printCellLocateData(UbloxATCellularInterfaceExt::CellLocData *);
void SendData(UbloxATCellularInterfaceExt *, char *, string);
string GetCellLocateData(UbloxATCellularInterfaceExt *);
// string getChangeStatusURL(string);
void isr_rising();
// void isr_falling();

InterruptIn button(P2_12);            //attaching interrupt to Button on Digital Pin 4
DigitalOut transmission_led(P2_1);    //transmission status led at Pin 5
DigitalOut ready_led(P2_0);           //Ready status led at Pin 3


int Cell_Loc_Count=0;
int cellDataReady=0;
int buttonPress=0;
char id[] = "C027";
float current_lat=0, current_lon=0;
string global_data_url, temp;

int main()
{
    int loop=0;
    UbloxATCellularInterfaceExt *interface = new UbloxATCellularInterfaceExt();
    GnssSerial gnssSerial;
    string update_data_url, status_url;
    char *loc_update;
    
    ready_led=1;
    wait_ms(300);
    ready_led=0;
    wait_ms(300);
    ready_led=1;
    wait_ms(100);
    ready_led=0;

    //INITIALIZATION
    
    // Powering up GNSS to assist with the Cell Locate bit
    gnssSerial.init();
    // Initializing Interrupt functions
    button.rise(&isr_rising);
    // button.fall(&isr_falling);

    printf("---System startup---\n");
    printf("-Registering to the Network-\n");
    printf("-Please wait up to 3 minutes-\n");
    if (interface->init(PIN)) 
    {
        ready_led=0;
        interface->set_credentials(APN, USERNAME, PASSWORD);
        printf("Registration successful...\n");
        printf("--Attempting connection to the packet network...\n");
        for (int x = 0; interface->connect() != 0; x++)
        {
            if (x > 0) 
                printf("Retrying! Please check the antenna and whether the APN is correct.\n");
        }
        printf("Successfully connected to the packet network...\n\n");
        ready_led=1;
        

        while (1)
        {        
            update_data_url = GetCellLocateData(interface);         //getting the encoded request URL for current Location data
            printf("Data URL = %s\n",update_data_url.c_str());
            // wait_ms(1000);
            if (cellDataReady && buttonPress==1)
            {
                loc_update = &update_data_url[0u];                      //converting it from a string to char *
                SendData(interface, loc_update, "post");                //Sending an HTTP post request with that URL
                buttonPress=0;
                transmission_led=0;
            }
            wait_ms(5000);
        }  
        
    } 
    else
    {
        printf("Unable to initialise the interface.\n");
        ready_led=0;
    }

}

string GetCellLocateData(UbloxATCellularInterfaceExt *interface)
{
    UbloxATCellularInterfaceExt::CellLocData data;
    int numRes;
    string ret_value;
    int retry_count=0;
    cellDataReady=0;

    // CELL LOCATE OPERATIONS (in a loop)
        printf("=== Cell Locate ===\n");
        
        while (cellDataReady==0) 
        {
            if (retry_count>6)
            {
                printf("\n\nWARNING : Obtaining Cell Locate data is taking too long... Consider restarting the device!\n\n");
            }
            if (retry_count>0)
            {
                printf("Invalid location data. Retry # %d\n\n", retry_count);
            }
            interface->cellLocSrvUdp();
            interface->cellLocConfig(1); // Deep scan mode
            printf("Sending Cell Locate request...\n");
            if (interface->cellLocRequest(UbloxATCellularInterfaceExt::CELL_HYBRID, 10, 100)) {
                // Wait for the response
                numRes = 0;
                for (int x = 0; (numRes == 0) && (x < 10); x++) 
                {
                    numRes = interface->cellLocGetRes();
                }
                
                if (numRes > 0) 
                {
                    interface->cellLocGetData(&data);
                    if (data.validData) 
                        ret_value = printCellLocateData(&data);
                } 
                else
                { 
                    printf("No response from Cell Locate server.\n");
                }
            }
            retry_count++;
            wait_ms(4000);
        }
    retry_count=0;
    return ret_value;
}

void SendData(UbloxATCellularInterfaceExt *interface, char *loc_update, string req_type)
{
    temp += loc_update;
    const char * url_path = temp.c_str();

    UbloxATCellularInterfaceExt::Error *err;
    int httpProfile;
    char buf[1024];
    printf("=== HTTP ===\n");
        printf("Performing HTTP POST on \"http://35.173.226.21/\"...\n");
        httpProfile = interface->httpAllocProfile();
        interface->httpSetTimeout(httpProfile, 30000);
        interface->httpSetPar(httpProfile, UbloxATCellularInterfaceExt::HTTP_IP_ADDRESS , "35.173.226.21");
        
        // Performing the HTTP command
        err = interface->httpCommand(httpProfile, (req_type=="post") ? UbloxATCellularInterfaceExt::HTTP_POST_DATA : UbloxATCellularInterfaceExt::HTTP_GET,
                                    (req_type=="post") ? "/coordinates" : url_path, NULL, (req_type=="post") ? loc_update : NULL, 0, NULL,
                                     (buf), sizeof (buf));
        if (err == NULL)
        {
            printf("HTTP POST on \"http://35.173.226.21/\" completed.  The response contained:\n"
                   "----------------------------------------------------------------------------------------\n%s"
                   "----------------------------------------------------------------------------------------\n", buf);
        }
        
        else 
        {
            printf("Unable to get valid data from \"http://35.173.226.21/\", "
                   "error class %d, error code %d.\n", err->eClass, err->eCode);
        }
}

static string printCellLocateData(UbloxATCellularInterfaceExt::CellLocData *pData)
{
    string loc;
    char timeString[25];

    printf("Cell Locate data:\n");
    if (strftime(timeString, sizeof(timeString), "%F %T", (const tm *) &(pData->time)) > 0) 
    {
        printf("  time:               %s\n", timeString);
    }
    printf("  longitude:          %.6f\n", pData->longitude);
    printf("  latitude:           %.6f\n", pData->latitude);
    printf("  altitude:           %d metre(s)\n", pData->altitude);
    switch (pData->sensor) {
        case UbloxATCellularInterfaceExt::CELL_LAST:
            printf("  sensor type:        last\n");
            loc = "last";
            break;
        case UbloxATCellularInterfaceExt::CELL_GNSS:
            printf("  sensor type:        GNSS\n");
            loc = "GNSS";
            break;
        case UbloxATCellularInterfaceExt::CELL_LOCATE:
            printf("  sensor type:        Cell Locate\n");
            loc = "Cell_Locate";
            break;
        case UbloxATCellularInterfaceExt::CELL_HYBRID:
            printf("  sensor type:        hybrid\n");
            loc = "hybrid";
            break;
        default:
            printf("  sensor type:        unknown\n");
            break;
    }
    printf("  uncertainty:        %d metre(s)\n", pData->uncertainty);
    printf("  speed:              %d metre(s)/second\n", pData->speed);
    printf("  direction:          %d degree(s)\n", pData->direction);
    printf("  vertical accuracy:  %d metre(s)/second\n", pData->speed);
    printf("  satellite(s) used:  %d\n", pData->svUsed);
    printf("Device Location: "
           "https://maps.google.com/?q=%.6f,%.6f\n", pData->latitude, pData->longitude);

    //Updating Global Variables for >0 check

    current_lat = pData->latitude;
    current_lon = pData->longitude;
    // printf("LATITUDE = %f\n",current_lat);
    // printf("LONGITUDE = %f\n",current_lon);
    if ((current_lat && current_lon)>0)
    {
        cellDataReady=1;        //Valid Cell data is ready
    }
    else
    {
        cellDataReady=0;        //Valid Cell data is NOT ready
    }
    //  //  //

    //Forming Request URL for a URL-encoded POST//

    char lat[12], lon[12];  //floats
    char vel[5], acc[5], alt[5];
    string data;

    sprintf(lat, "%.6f", pData->latitude);
    sprintf(lon, "%.6f", pData->longitude);
    sprintf(vel, "%d", pData->speed);
    sprintf(acc, "%d", pData->direction);
    sprintf(alt, "%d", pData->altitude);

    string device_id = "id=";
    device_id += id;
    string latitude = "&lat=";
    latitude += lat; 
    string longitude = "&lon=";
    longitude += lon; 
    string sensor = "&loc=";
    sensor += loc;  
    string speed = "&speed=";
    speed += vel;
    string acceleration = "&acc=";
    acceleration += acc;
    string altitude = "&alt=";
    altitude += alt;
    data = device_id+latitude+longitude+sensor+speed+acceleration+altitude;
    global_data_url = data;
    return data;
    //  //  //
}

void shutdown_sequence(UbloxATCellularInterfaceExt *interface,GnssSerial gnssSerial)
{
    printf("Initiating shutdown sequence for the GSM & GNSS modules...\n");
    gnssSerial.powerOff();
    interface->disconnect();
    interface->deinit();
    printf("Stopped.\n");
}

void isr_rising()
{
    // printf("Button pressed.\n");
    buttonPress=1;
    transmission_led=1;
}


/*
        //Figure this out later
        //Put this in an interrupt for isr_falling and an opposite in isr_rising
        //isr_falling should stop the HTTP post requests from happening any further and send a "no"
        //isr_rising should start the HTTP post requests again & send "yes"
        //starting and stopping can be done with a flag
        
        // status_url = getChangeStatusURL("no");                  //getting the encoded request URL for status change
        // printf("Status URL = %s\n",status_url.c_str());
        // char* status_update = &status_url[0u];                  //coverting it from a string to char *
        // SendData(interface, status_update, "get");              //Sending an HTTP get request with that URL
*/

/*
    // void SendData(UbloxATCellularInterfaceExt *interface, char *loc_update, string req_type)
    // {
    //     temp += loc_update;
    //     const char * url_path = temp.c_str();

    //     UbloxATCellularInterfaceExt::Error *err;
    //     int httpProfile;
    //     char buf[1024];
    //     printf("=== HTTP ===\n");
    //         printf("Performing HTTP POST on \"http://35.173.226.21/\"...\n");
    //         httpProfile = interface->httpAllocProfile();
    //         interface->httpSetTimeout(httpProfile, 30000);
    //         interface->httpSetPar(httpProfile, UbloxATCellularInterfaceExt::HTTP_IP_ADDRESS , "35.173.226.21");
            
    //         // Performing the HTTP command
    //         err = interface->httpCommand(httpProfile, (req_type=="post") ? UbloxATCellularInterfaceExt::HTTP_POST_DATA : UbloxATCellularInterfaceExt::HTTP_GET,
    //                                     (req_type=="post") ? "/coordinates" : url_path, NULL, (req_type=="post") ? loc_update : NULL, 0, NULL,
    //                                      (buf), sizeof (buf));
    //         if (err == NULL)
    //         {
    //             printf("HTTP POST on \"http://35.173.226.21/\" completed.  The response contained:\n"
    //                    "----------------------------------------------------------------------------------------\n%s"
    //                    "----------------------------------------------------------------------------------------\n", buf);
    //         }
            
    //         else 
    //         {
    //             printf("Unable to get valid data from \"http://35.173.226.21/\", "
    //                    "error class %d, error code %d.\n", err->eClass, err->eCode);
    //         }
    // }
*/

/*
    // string getChangeStatusURL(string status)
    // {
    //     string device_id = "id=";
    //     device_id += id;
    //     string val = "&active=";
    //     val += status;
    //     string value_string = device_id + val;
    //     return value_string;
    // }
*/