/*
 * Generated by erpcgen 1.9.1 on Fri Sep  9 09:53:34 2022.
 *
 * AUTOGENERATED - DO NOT EDIT
 */

#if !defined(_GenericDataOperations_EvbToPc_server_h_)
    #define _GenericDataOperations_EvbToPc_server_h_

    #ifdef __cplusplus
        #include "erpc_codec.hpp"
        #include "erpc_server.hpp"
extern "C" {
        #include "GenericDataOperations_EvbToPc.h"
        #include <stdbool.h>
        #include <stdint.h>
}

        #if 10901 != ERPC_VERSION_NUMBER
            #error "The generated shim code version is different to the rest of eRPC code."
        #endif

/*!
 * @brief Service subclass for evb_to_pc.
 */
class evb_to_pc_service : public erpc::Service {
  public:
    evb_to_pc_service() : Service(kevb_to_pc_service_id) {}

    /*! @brief Call the correct server shim based on method unique ID. */
    virtual erpc_status_t
    handleInvocation(uint32_t methodId, uint32_t sequence, erpc::Codec *codec,
                     erpc::MessageBufferFactory *messageFactory);

  private:
    /*! @brief Server shim for ns_rpc_data_sendBlockToPC of evb_to_pc interface. */
    erpc_status_t
    ns_rpc_data_sendBlockToPC_shim(erpc::Codec *codec, erpc::MessageBufferFactory *messageFactory,
                                   uint32_t sequence);

    /*! @brief Server shim for ns_rpc_data_fetchBlockFromPC of evb_to_pc interface. */
    erpc_status_t
    ns_rpc_data_fetchBlockFromPC_shim(erpc::Codec *codec,
                                      erpc::MessageBufferFactory *messageFactory,
                                      uint32_t sequence);

    /*! @brief Server shim for ns_rpc_data_computeOnPC of evb_to_pc interface. */
    erpc_status_t
    ns_rpc_data_computeOnPC_shim(erpc::Codec *codec, erpc::MessageBufferFactory *messageFactory,
                                 uint32_t sequence);

    /*! @brief Server shim for ns_rpc_data_remotePrintOnPC of evb_to_pc interface. */
    erpc_status_t
    ns_rpc_data_remotePrintOnPC_shim(erpc::Codec *codec, erpc::MessageBufferFactory *messageFactory,
                                     uint32_t sequence);
};

extern "C" {
    #else
        #include "GenericDataOperations_EvbToPc.h"
    #endif // __cplusplus

typedef void *erpc_service_t;

erpc_service_t
create_evb_to_pc_service(void);

    #if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
void
destroy_evb_to_pc_service(erpc_service_t service);
    #elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
void
destroy_evb_to_pc_service(void);
    #else
        #warning "Unknown eRPC allocation policy!"
    #endif

    #ifdef __cplusplus
}
    #endif // __cplusplus

#endif // _GenericDataOperations_EvbToPc_server_h_
