/*  nTox_win32.c
 *
 *  Textual frontend for Tox - Windows version
 *
 *  Copyright (C) 2013 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *  
 */

#include "nTox_win32.h"
#include "misc_tools.h"

#include <process.h>

uint8_t pending_requests[256][CLIENT_ID_SIZE];
uint8_t num_requests;

char line[STRING_LENGTH];
char users_id[200];

void do_header()
{
    system("cls");
    printf(users_id);
    printf("\n---------------------------------");
    printf("\n[i] commands: /f ID (to add friend), /m friendnumber message  (to send message), /s status (to change status), /n nick (to change nickname), /l (lists friends), /d friendnumber (deletes friend), /q (to quit), /r (reset screen)");
    printf("\n---------------------------------");
}

void print_request(uint8_t *public_key, uint8_t *data, uint16_t length)
{
    printf("\n\n[i] received friend request with message\n");
    printf((char *)data);
    char numchar[100];
    sprintf(numchar, "\n\n[i] accept request with /a %u\n\n", num_requests);
    printf(numchar);
    memcpy(pending_requests[num_requests], public_key, CLIENT_ID_SIZE);
    ++num_requests;
}

void print_message(int friendnumber, uint8_t * string, uint16_t length)
{
    char name[MAX_NAME_LENGTH];
    getname(friendnumber, (uint8_t*)name);
    char msg[100+length+strlen(name)+1];
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    char* temp = asctime(timeinfo);
    size_t len = strlen(temp);
    temp[len-1]='\0';
    sprintf(msg, "\n[%d] %s <%s> %s\n\n", friendnumber, temp, name, string); // timestamp
    printf(msg);
}

void print_nickchange(int friendnumber, uint8_t *string, uint16_t length)
{
    char name[MAX_NAME_LENGTH];
    getname(friendnumber, (uint8_t*)name);
    char msg[100+length];
    sprintf(msg, "\n\n[i] [%d] %s is now known as %s.\n\n", friendnumber, name, string);
    printf(msg);
}

void print_statuschange(int friendnumber, uint8_t *string, uint16_t length) {
    char name[MAX_NAME_LENGTH];
    getname(friendnumber, (uint8_t*)name);
    char msg[100+length+strlen(name)+1];
    sprintf(msg, "\n\n[i] [%d] %s's status changed to %s.\n", friendnumber, name, string);
    printf(msg);
}

void load_key() 
{
    FILE *data_file = NULL;

    if ((data_file = fopen("data", "r"))) {
        fseek(data_file, 0, SEEK_END);
        int size = ftell(data_file);
        fseek(data_file, 0, SEEK_SET);
        uint8_t data[size];

        if(fread(data, sizeof(uint8_t), size, data_file) != size) {
            printf("\n[i] Could not read the data file. Exiting.");
            exit(1);
        }

        Messenger_load(data, size);
    } else {
        int size = Messenger_size();
        uint8_t data[size];
        Messenger_save(data);
        data_file = fopen("data", "w");

        if(fwrite(data, sizeof(uint8_t), size, data_file) != size) {
            printf("\n[i] Could not write data to file. Exiting.");
            exit(1);
        }
    }

    fclose(data_file);
}

void line_eval(char* line)
{
    if(line[0] == '/') {
        /* Add friend */
        if(line[1] == 'f') {
            int i;
            char temp_id[128];
            for (i=0; i<128; i++) 
                temp_id[i] = line[i+3];
            int num = m_addfriend(hex_string_to_bin(temp_id), (uint8_t*)"Install Gentoo", sizeof("Install Gentoo"));
            char numstring[100];
            sprintf(numstring, "\n[i] added friend %d\n\n", num);
            printf(numstring);
        }

        else if (line[1] == 'r') {
            do_header();
            printf("\n\n");
        }

        else if (line[1] == 'l') {
            printf("\n[i] Friend List | Total: %d\n\n", getnumfriends());

            int i;

            for (i=0; i < getnumfriends(); i++) {
                char name[MAX_NAME_LENGTH];
                getname(i, (uint8_t*)name);
                printf("[%d] %s\n\n", i, (uint8_t*)name);
            }
        }

        else if (line[1] == 'd') {
            size_t len = strlen(line);
            char numstring[len-3];
            int i;
            for (i=0; i<len; i++) {
                if (line[i+3] != ' ') {
                    numstring[i] = line[i+3];
                }
            }
            int num = atoi(numstring);
            m_delfriend(num);
        }
        /* Send message to friend */
        else if (line[1] == 'm') {
            int i;
            size_t len = strlen(line);
            char numstring[len-3];
            char message[len-3];
            for (i=0; i<len; i++) {
                if (line[i+3] != ' ') {
                    numstring[i] = line[i+3];
                } else {
                    int j;
                    for (j=i+1; j<len; j++)
                        message[j-i-1] = line[j+3];
                    break;
                }
            }
            int num = atoi(numstring);
            if(m_sendmessage(num, (uint8_t*) message, sizeof(message)) != 1) {
                printf("\n[i] could not send message: %s\n", message);
            } else {
                //simply for aesthetics
                printf("\n");
            }
        }

        else if (line[1] == 'n') {
            uint8_t name[MAX_NAME_LENGTH];
            int i = 0;
            size_t len = strlen(line);
            for (i=3; i<len; i++) {
                if (line[i] == 0 || line[i] == '\n') break;
                name[i - 3] = line[i];
            }
            name[i - 3] = 0;
            setname(name, i);
            char numstring[100];
            sprintf(numstring, "\n[i] changed nick to %s\n\n", (char*)name);
            printf(numstring);
        }

        else if (line[1] == 's') {
            uint8_t status[MAX_USERSTATUS_LENGTH];
            int i = 0;
            size_t len = strlen(line);
            for (i=3; i<len; i++) {
                if (line[i] == 0 || line[i] == '\n') break;
                status[i - 3] = line[i];
            }
            status[i - 3] = 0;
            m_set_userstatus(status, strlen((char*)status));
            char numstring[100];
            sprintf(numstring, "\n[i] changed status to %s\n\n", (char*)status);
            printf(numstring);
        }

        else if (line[1] == 'a') {
            uint8_t numf = atoi(line + 3);
            char numchar[100];
            sprintf(numchar, "\n[i] friend request %u accepted\n\n", numf);
            printf(numchar);
            int num = m_addfriend_norequest(pending_requests[numf]);
            sprintf(numchar, "\n[i] added friendnumber %d\n\n", num);
            printf(numchar);
        }
        /* EXIT */
        else if (line[1] == 'q') { 
            exit(EXIT_SUCCESS);
        }
    } 
    
    else {
        //nothing atm
    }
}

void get_input()
{
    while(1) {
        fgets(line, STRING_LENGTH, stdin);
        line_eval(line);
        strcpy(line, "");
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("[!] Usage: %s [IP] [port] [public_key] <nokey>\n", argv[0]);
        exit(0);
    }

    if (initMessenger() == -1) {
        printf("initMessenger failed");
        exit(0);
    }
    
    if (argc > 4) {
        if(strncmp(argv[4], "nokey", 6) < 0) {
        }
    } else {
        load_key();
    }
    
    m_callback_friendrequest(print_request);
    m_callback_friendmessage(print_message);
    m_callback_namechange(print_nickchange);
    m_callback_userstatus(print_statuschange);

    char idstring1[32][5];
    char idstring2[32][5];
    uint32_t i;
    for(i = 0; i < 32; i++)
    {
        if(self_public_key[i] < 16)
            strcpy(idstring1[i],"0");
        else 
            strcpy(idstring1[i], "");
        sprintf(idstring2[i], "%hhX",self_public_key[i]);
    }
    strcpy(users_id,"[i] your ID: ");
    for (i=0; i<32; i++) {
        strcat(users_id,idstring1[i]);
        strcat(users_id,idstring2[i]);
    }

    do_header();

    IP_Port bootstrap_ip_port;
    bootstrap_ip_port.port = htons(atoi(argv[2]));
    int resolved_address = resolve_addr(argv[1]);
    if (resolved_address != -1)
        bootstrap_ip_port.ip.i = resolved_address;
    else 
        exit(1);
    
    DHT_bootstrap(bootstrap_ip_port, hex_string_to_bin(argv[3]));

    int c;
    int on = 0;

    _beginthread(get_input, 0, NULL);

    while(1) {
        if (on == 1 && DHT_isconnected() == -1) {
            printf("\n---------------------------------");
            printf("\n[i] Disconnected from the DHT");
            printf("\n---------------------------------\n\n");
            on = 0;
        }

        if (on == 0 && DHT_isconnected()) {
            printf("\n[i] Connected to DHT");
            printf("\n---------------------------------\n\n");
            on = 1;
        }

        doMessenger();
        Sleep(1);
    }

    return 0;
}