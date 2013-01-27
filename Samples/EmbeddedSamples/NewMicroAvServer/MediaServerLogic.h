#ifndef MEDIASERVERLOGIC_H
#define MEDIASERVERLOGIC_H

/*
 *	This provides the baseline infrastructure for a
 *	minimal media server that implements the content enumeration.
 *
 *	Implemented features are as follows.
 *		CDS: Browse, GetSystemUpdateID, GetSearchCapabilities, GetSortCapabilities
 *		CM: GetCurrentConnectionIDs, GetCurrentConnectionInfo, GetProtocolInfo
 *		HTTP 1.0 webserver with RANGE support
 *
 *
 *	This module provides the means for an application to respond to 
 *	ContentDirectory requests related to content discovery. Specifically,
 *	the two relevant actions are Browse and Search. 
 *
 *	Architecturally, the module sits directly on top of a DeviceBuilder-generated
 *	microstack and routes the callback sinks for Browse and Search to methods
 *	defined in the module. The module defines two callbacks for the upper software
 *	layer: MSL_Callback_OnBrowse, MSL_Callback_OnSearch. The appropriate callback 
 *	executes when the upper-application layer needs to respond to a browse or search request.
 *
 *	-Each method call requires the object returned from CreateMediaServer.
 *
 *	-All strings for the interface are assumed to be UTF8 compliant. This means
 *	 that string arguments in callbacks will be sent in their UTF8-form
 *	 and that all strings sent in response to a CP's action are properly
 *	 encoded by the application layer in UTF8 form. 
 *
 */


/************************************************************************************/
/* START SECTION - error messages */
#define MSL_ErrorMsg_ConnectionDoesNotExist "Specified connection does not exist."

/* END SECTION - error messages */
/************************************************************************************/

enum MSL_ErrorCodes
{
	MSL_Error_ConnectionDoesNotExist		= 706
};

enum MSL_Enum_SrcProtInfo
{
	MSL_SrcProtInfo_HTTPGET_STAR_STAR_STAR	= 0
};

/*
 *	Provides statistical info on the calls made on the media server.
 */
struct MSL_Stats
{
	unsigned int		Count_Browse;
	unsigned int		Count_GetSystemUpdateID;
	unsigned int		Count_GetSearchCapabilities;
	unsigned int		Count_GetSortCapabilities;
	unsigned int		Count_GetCurrentConnectionIDs;
	unsigned int		Count_GetCurrentConnectionInfo;
	unsigned int		Count_GetProtocolInfo;
};


/*
 *	Encapsulates input arguments for a Browse request.
 */
struct MSL_BrowseArgs
{
	/*
	 *	A MediaServer token. Such an object can be matched with the application's
	 *	list of active MediaServer objects, each returned from MSL_CreateMediaServer().
	 */
	void *MediaServerObject;

	/*
	 *	Application can use this token when calling UpnpResponse_xxx methods.
	 */
	void *UpnpToken;

	/*
	 *	ObjectID specified by the control point.
	 */
	char *ObjectID;

	/*
	 *	Nonzero value indicates that the CP wants the children of the
	 *	specified objectID. Otherwise, the CP wants the metadata for
	 *	the specified object.
	 */
	int BrowseDirectChildren;

	/*
	 *	Metadata filter settings. Comma-delimited list of [namespace_prefix]:[tag_name] strings, 
	 *	that describe what CDS metadata to return in the response. In this framework,
	 *	the application layer is responsible for enforcing metadata filtering.
	 */
	char *Filter;

	/*
	 *	The index of the first media object to return. Zero-based value.
	 *	Only applicable when BrowseDirectChildren is nonzero.
	 */
	unsigned int StartingIndex;
	
	/*
	 *	The maximum number of media objects to return. Zero means return all media objects.
	 *	Only applicable when BrowseDirectChildren is nonzero.
	 */
	unsigned int RequestedCount;

	/*
	 *	SortCriteria strings have the following form:
	 *	[+/-][namespace prefix]:[tag name],[+/-][namespace prefix]:[tag name],...	 
	 */
	char *SortCriteria;

	/*
	 *	Reserved: contains the argument name to return DIDL-Lite results.
	 */
	void *_ReservedDidlOutArgname;

	/*
	 *	Miscellaneous field for application-level logic.
	 *	Application must free this field if it allocates to it.
	 */
	void *UserObject;
};


/*
 *	This application-level assigned function pointer, causes
 *	the callback sink to execute when the UPnP layer 
 *	receives a request for a item or container's metadata.
 *	Applications should use the MSL_DeallocateBrowseArgs()
 *	to free resources held by browseArgs.
 */
extern void (*MSL_Callback_OnBrowse) (struct MSL_BrowseArgs *browseArgs);

/*
 *	This application-level assigned function pointer, causes
 *	the callback sink to execute whenever the statistics for the media server changes.
 */
extern void (*MSL_OnStatsChanged) (struct MS_Stats *stats);




/*
 *	This method creates the MediaServer object that abstracts state information
 *	for a MediaServer. 
 */
void *MSL_CreateMediaServer(void *chain, void *upnpStack, void* lifetimeMonitor);

/*
 *	Allows applications to deallocate the MSL_BrowseArgs object
 *	that was sent in MSL_Callback_OnBrowse.
 *
 *		browseArgs				: Address of the browse argument struct specified in the callback.
 *								  Upon return, *browseArgs will be NULL.
 */
void MSL_DeallocateBrowseArgs(struct MSL_BrowseArgs **browseArgs);

/*
 *	Application-level code can call this method within their
 *	implementation of MSL_Callback_OnBrowse when they need to respond to the
 *	control point with an error.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		errorCode				: This value can be one of the values in MSL_ErrorCodes
 *								: or it can be be a vendor-defined value between the range
 *								: of [800,899] inclusive.
 *
 *		errorMsg				: This is the custom-message to include with the error code.
 *								: The string must be encoded with UTF8 compliance.
 *
 */
void MSL_ForResponse_RespondError(struct MSL_BrowseArgs *browseArgs, int errorCode, const char *errorMsg);

/*
 *	Application-level code can call this method within their
 *	implementation of MSL_Callback_OnBrowse when they need to begin the response to the
 *	control point. This method should be called before calling 
 *	MSL_ForResponse_RespondBrowse_ResultArgument or MSL_ForResponse_RespondBrowse_FinishResponse.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		sendDidlHeader			: If nonzero, the DIDL-Lite header is sent. If nonzero,
 *								: the application is responsible for sending that header
 *								: through MSL_ForResponse_RespondBrowse_ResultArgument.
 */
void MSL_ForResponse_RespondBrowse_StartResponse(struct MSL_BrowseArgs *browseArgs, int sendDidlHeader);

/*
 *	Application-level code can call this method within their
 *	implementation of MSL_Callback_OnBrowse when they need to respond to the
 *	control point with data in the Result argument. This method must
 *	be called after MSL_ForResponse_RespondBrowse_StartResponse but
 *	before MSL_ForResponse_RespondBrowse_FinishResponse.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		xmlEscapedUtf8Didl		: DIDL-Lite response data, where the data is properly XML-escaped and also in UTF8 encoding.
 *								: The upper application layer can call this an arbitrary number of times, so the
 *								: upper application layer can decide between making fewer calls with larger
 *								: DIDL-Lite data blocks or more calls with smaller DIDL-Lite data blocks.
 *
 *		didlSize				: The number of bytes in xmlEscapedUtf8Didl, not including a trailing null-terminator.
 */
void MSL_ForResponse_RespondBrowse_ResultArgument(struct MSL_BrowseArgs *browseArgs, const char *xmlEscapedUtf8Didl, int didlSize);

/*
 *	Application-level code can call this method within their
 *	implementation of MSL_Callback_OnBrowse when they need to finish the
 *	response to a control point.
 *
 *		browseArgs				: browse argument struct specified in a callback.
 *
 *		sendDidlFooter			: If nonzero, the DIDL-Lite footer is sent. If nonzero,
 *								: the application will have sent the DIDL-Lite footer in a previous
 *								: call to MSL_ForResponse_RespondBrowse_ResultArgument.
 *
 *		numberReturned			: the number of DIDL-Lite elements returned in the response.
 *
 *		totalMatches			: the number of DIDL-Lite elements that could be returned given the request.
 *
 *		updateID				: the updateID of the container. Zero, if respond to a BrowseMetadata request on an item.
 */
void MSL_ForResponse_RespondBrowse_FinishResponse(struct MSL_BrowseArgs *browseArgs, int sendDidlFooter, unsigned int numberReturned, unsigned int totalMatches, unsigned int updateID);

/*
 *	Applications call this method to increment the mediaserver's reported
 *	value for the SystemUpdateID state variable.
 *
 *		mslObj					: The object returned from MSL_CreateMediaServer
 */
void MSL_IncrementSystemUpdateID(void *mslObj);

/*
 *	Applications call this method to instruct the mediaserver's reported
 *	UpdateID values for individual container objects. This method
 *	will automatically moderate the delivery of events for the
 *	ContainerUpdateID state variable.
 *
 *	Although this method automatically moderates the delivery of the UPnP event,
 *	applications should avoid calling this method repetitively with the same
 *	containerID in a short period of time. The primary performance issue with this
 *	method is that it has to prevent duplicate containerID entries in the
 *	state variable.
 *
 *		mslObj					: The object returned from MSL_CreateMediaServer
 *
 *		containerID				: the ID of the container that changed
 *
 *		containerUpdateID		: the new updateID of the container that changed
 */
void MSL_UpdateContainerID(void *mslObj, const char *containerID, unsigned int containerUpdateID);



#endif

