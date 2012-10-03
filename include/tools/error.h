/*
 * error.h
 *
 *  Created on: 03.10.2012
 *      Author: savop
 */

#ifndef ERROR_H_
#define ERROR_H_

/****************************************************
 *
 * Known Errors
 *
 ****************************************************/

/* Errors 401-404, 501 are already defined in
 * Intel SDK, however 403 MUST NOT USED.
 */

/****** 600 Common Action Errors ******/

#define UPNP_SOAP_E_ARGUMENT_INVALID            600
#define UPNP_SOAP_E_ARGUMENT_OUT_OF_RANGE       601
#define UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED      602
#define UPNP_SOAP_E_OUT_OF_MEMORY               603
#define UPNP_SOAP_E_HUMAN_INTERVENTION          604
#define UPNP_SOAP_E_STRING_TO_LONG              605
#define UPNP_SOAP_E_NOT_AUTHORIZED              606
#define UPNP_SOAP_E_SIGNATURE_FAILURE           607
#define UPNP_SOAP_E_SIGNATURE_MISSING           608
#define UPNP_SOAP_E_NOT_ENCRYPTED               609
#define UPNP_SOAP_E_INVALID_SEQUENCE            610
#define UPNP_SOAP_E_INVALID_CONTROL_URL         611
#define UPNP_SOAP_E_NO_SUCH_SESSION             612

/****** 700 Action specific Errors ******/

#define UPNP_CDS_E_NO_SUCH_OBJECT               701
#define UPNP_CDS_E_INVALID_CURRENT_TAG          702
#define UPNP_CDS_E_INVALID_NEW_TAG              703
#define UPNP_CDS_E_REQUIRED_TAG                 704
#define UPNP_CDS_E_READ_ONLY_TAG                705
#define UPNP_CDS_E_PARAMETER_MISMATCH           706
#define UPNP_CDS_E_INVALID_SEARCH_CRITERIA      708
#define UPNP_CDS_E_INVALID_SORT_CRITERIA        709
#define UPNP_CDS_E_NO_SUCH_CONTAINER            710
#define UPNP_CDS_E_RESTRICTED_OBJECT            711
#define UPNP_CDS_E_BAD_METADATA                 712
#define UPNP_CDS_E_RESTRICTED_PARENT            713
#define UPNP_CDS_E_NO_SUCH_SOURCE_RESOURCE      714
#define UPNP_CDS_E_RESOURCE_ACCESS_DENIED       715
#define UPNP_CDS_E_TRANSFER_BUSY                716
#define UPNP_CDS_E_NO_SUCH_FILE_TRANSFER        717
#define UPNP_CDS_E_NO_SUCH_DESTINATION_RESOURCE 718
#define UPNP_CDS_E_DEST_RESOURCE_ACCESS_DENIED  719
#define UPNP_CDS_E_CANT_PROCESS_REQUEST         720

#define UPNP_CMS_E_INCOMPATIBLE_PROTOCOL_INFO   701
#define UPNP_CMS_E_INCOMPATIBLE_DIRECTIONS      702
#define UPNP_CMS_E_INSUFFICIENT_RESOURCES       703
#define UPNP_CMS_E_LOCAL_RESTRICTIONS           704
#define UPNP_CMS_E_ACCESS_DENIED                705
#define UPNP_CMS_E_INVALID_CONNECTION_REFERENCE 706
#define UPNP_CMS_E_NOT_IN_NETWORK               707

#endif /* ERROR_H_ */
