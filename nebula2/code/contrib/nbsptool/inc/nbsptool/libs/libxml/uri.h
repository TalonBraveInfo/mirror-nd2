/**
 * uri.c: library of generic URI related routines 
 *
 * Reference: RFC 2396
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#ifndef __XML_URI_H__
#define __XML_URI_H__

#include <nbsptool/libs/libxml/xmlversion.h>
#include <nbsptool/libs/libxml/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * xmlURI:
 *
 * A parsed URI reference. This is a struct containing the various fields
 * as described in RFC 2396 but separated for further processing.
 */
typedef struct _xmlURI xmlURI;
typedef xmlURI *xmlURIPtr;
struct _xmlURI {
    char *scheme;    /* the URI scheme */
    char *opaque;    /* opaque part */
    char *authority;    /* the authority part */
    char *server;    /* the server part */
    char *user;        /* the user part */
    int port;        /* the port number */
    char *path;        /* the path string */
    char *query;    /* the query string */
    char *fragment;    /* the fragment identifier */
    int  cleanup;    /* parsing potentially unclean URI */
};

/*
 * This function is in tree.h:
 * xmlChar *    xmlNodeGetBase    (xmlDocPtr doc,
 *                               xmlNodePtr cur);
 */
XMLPUBFUN xmlURIPtr XMLCALL    
        xmlCreateURI        (void);
XMLPUBFUN xmlChar * XMLCALL    
        xmlBuildURI        (const xmlChar *URI,
                                 const xmlChar *base);
XMLPUBFUN xmlURIPtr XMLCALL    
        xmlParseURI        (const char *str);
XMLPUBFUN int XMLCALL        
        xmlParseURIReference    (xmlURIPtr uri,
                     const char *str);
XMLPUBFUN xmlChar * XMLCALL    
        xmlSaveUri        (xmlURIPtr uri);
XMLPUBFUN void XMLCALL        
        xmlPrintURI        (FILE *stream,
                     xmlURIPtr uri);
XMLPUBFUN xmlChar * XMLCALL       
        xmlURIEscapeStr         (const xmlChar *str,
                      const xmlChar *list);
XMLPUBFUN char * XMLCALL        
        xmlURIUnescapeString    (const char *str,
                     int len,
                     char *target);
XMLPUBFUN int XMLCALL        
        xmlNormalizeURIPath    (char *path);
XMLPUBFUN xmlChar * XMLCALL    
        xmlURIEscape        (const xmlChar *str);
XMLPUBFUN void XMLCALL        
        xmlFreeURI        (xmlURIPtr uri);
XMLPUBFUN xmlChar* XMLCALL    
        xmlCanonicPath        (const xmlChar *path);

#ifdef __cplusplus
}
#endif
#endif /* __XML_URI_H__ */
