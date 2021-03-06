#pragma once

#include "FibonacciHeap.h"
#include "MCCIRequestBanks.h"
#include "MCCISchema.h"
#include "MCCIServerNetworking.h"
#include "MCCIRevisionSet.h"
#include "MCCITime.h"
#include "MCCITypes.h"
#include <map>
#include <vector>
#include <sqlite3.h>
#include <ostream>
//#include <unordered_map> // replace with boost?


// the settings for operating a MCCI server
typedef struct
{
    MCCI_NODE_ADDRESS_T my_node_address;
    
    unsigned int max_local_requests;
    unsigned int max_remote_requests;
    unsigned int max_clients;

    unsigned int bank_size_host;
    unsigned int bank_size_var;
    unsigned int bank_size_hostvar;
    unsigned int bank_size_varrev_var;
    unsigned int bank_size_varrev_rev;
    unsigned int bank_size_remote_hostvar;
    unsigned int bank_size_remote_rev;
    
    CMCCISchema* schema;
    CMCCIRevisionSet* revisionset;

} SMCCIServerSettings;

ostream& operator<<(ostream &out, SMCCIServerSettings const &rhs);


/**
   This class is the logical component of the MCCI system's packet request & delivery system.
 */
class CMCCIServer
{
    
  protected:
    SMCCIServerSettings m_settings;
    
    vector<SMCCIDataPacket*> m_working_set; // current values of stuff

    AllRequestBank              m_bank_all;
    HostRequestBank             m_bank_host;
    VariableRequestBank         m_bank_var;
    HostVariableRequestBank     m_bank_hostvar;
    RemoteRevisionRequestBank   m_bank_remote;
    VariableRevisionRequestBank m_bank_varrev;

    CMCCIServerNetworking* m_networking;
    CMCCITime* m_time;
    bool m_external_time;
    
  public:
    CMCCIServer(CMCCITime* time, CMCCIServerNetworking* networking, SMCCIServerSettings settings);
    CMCCIServer(const CMCCIServer&);
    ~CMCCIServer();

    // output operator
    friend ostream& operator<<(ostream &out, CMCCIServer const &rhs);

    // string version of output
    string summary() const;

    // number of open requests
    int request_count() const;
    
    // accept a request packet, and put its contents in the appropriate structures, responding accordingly
    void process_request(MCCI_CLIENT_ID_T requestor_id,
                         const SMCCIRequestPacket* input,
                         SMCCIResponsePacket* response);

    // accept a data packet, giving a reference to its contents to all necessary subscriber
    void process_data(MCCI_CLIENT_ID_T provider_id,
                      const SMCCIDataPacket* input);

    // accept a production packet
    void process_production(MCCI_CLIENT_ID_T provider_id,
                            const SMCCIProductionPacket* input,
                            SMCCIAcceptancePacket* output);

    // tell the client how many requests it is allowed to make
    unsigned int client_free_requests_local(MCCI_CLIENT_ID_T client_id) const;
    
    unsigned int client_free_requests_remote(MCCI_CLIENT_ID_T client_id) const;

    // remove all expired requests and update the outstanding_requests counters appropriately
    void enforce_timeouts();

    // remove all requests forz a specific packet that was delivered
    void enforce_fulfillment(const SMCCIDataPacket* delivered);
    
    // return the settings
    SMCCIServerSettings get_settings() const { return m_settings; }
    
  protected:

    void set_free_requests(SMCCIResponsePacket* response, MCCI_CLIENT_ID_T requestor_id) const
    {
        response->requests_remaining_local  = client_free_requests_local(requestor_id);
        response->requests_remaining_remote = client_free_requests_remote(requestor_id);
    }
    
    // whether an address is equivalent to "localhost"
    bool is_my_address(MCCI_NODE_ADDRESS_T address) const
    { return 0 == address || address == m_settings.my_node_address; };
    
    // whether a variable id has delivered its first value
    bool is_in_working_set(MCCI_VARIABLE_T variable_id) const
    {
        unsigned int idx = m_settings.schema->ordinality_of_variable(variable_id);
        return NULL != m_working_set.at(idx);
    }

    SMCCIDataPacket* get_working_variable(MCCI_VARIABLE_T variable_id)
    {
        unsigned int idx = m_settings.schema->ordinality_of_variable(variable_id);
        return m_working_set.at(idx);
    }

    void set_working_variable(MCCI_VARIABLE_T variable_id, SMCCIDataPacket* v)
    {
        unsigned int idx = m_settings.schema->ordinality_of_variable(variable_id);
        if (m_working_set[idx]) delete m_working_set[idx];
        m_working_set[idx] = v;
    }
    
    // whether a request has one of the 4 possible input combinations that makes it wrong
    bool is_rejectable_request(const SMCCIRequestPacket* input) const;

    // slave to process_request, for the cases that involve forwarding
    void process_forwardable_request(MCCI_CLIENT_ID_T requestor_id,
                                     const SMCCIRequestPacket* input,
                                     SMCCIResponsePacket* response);

    // add a client to the list of recipients for all data packets
    void subscribe_promiscuous(MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout);

    // add a client to the list of recipients for packets of given variable_id
    void subscribe_to_variable(MCCI_CLIENT_ID_T client_id,
                               MCCI_TIME_T timeout,
                               MCCI_VARIABLE_T variable_id);
    
    // add a client to the list of recipients for packets from given host
    void subscribe_to_host(MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout,
                           MCCI_NODE_ADDRESS_T node_address);

    // add a client to the list of recipients for packets from given host and variable ID
    void subscribe_to_host_var(MCCI_CLIENT_ID_T client_id,
                               MCCI_TIME_T timeout,
                               MCCI_NODE_ADDRESS_T node_address,
                               MCCI_VARIABLE_T variable_id);

    //add a client to the list of receipents for a specific packet from this host
    void subscribe_specific(MCCI_CLIENT_ID_T client_id,
                            MCCI_TIME_T timeout,
                            MCCI_VARIABLE_T variable_id,
                            MCCI_REVISION_T revision);
    
    // add a client to the list of recipients for a specific packet from a remote host
    void subscribe_specific_remote(MCCI_CLIENT_ID_T client_id,
                                   MCCI_TIME_T timeout,
                                   MCCI_NODE_ADDRESS_T node_address,
                                   MCCI_VARIABLE_T variable_id,
                                   MCCI_REVISION_T revision);

    
    // check client subscription to variable
    bool bank_contains_variable(MCCI_CLIENT_ID_T client_id,
                                MCCI_VARIABLE_T variable_id) const;
    
    // check client subscription to host
    bool bank_contains_host(MCCI_CLIENT_ID_T client_id, 
                            MCCI_NODE_ADDRESS_T node_address) const;

    // check client subscription to host / var
    bool bank_contains_host_var(MCCI_CLIENT_ID_T client_id,
                                MCCI_NODE_ADDRESS_T node_address,
                                MCCI_VARIABLE_T variable_id) const;

    // check client subscription to a specific local variable + revision
    bool bank_contains_specific(MCCI_CLIENT_ID_T client_id,
                                MCCI_VARIABLE_T variable_id,
                                MCCI_REVISION_T revision) const;
    
    // check client subscrition to a specific remote host + var + revision
    bool bank_contains_specific_remote(MCCI_CLIENT_ID_T client_id,
                                       MCCI_NODE_ADDRESS_T node_address,
                                       MCCI_VARIABLE_T variable_id,
                                       MCCI_REVISION_T revision) const;
    

};


