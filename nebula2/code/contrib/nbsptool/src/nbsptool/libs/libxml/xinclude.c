/*
 * xinclude.c : Code to implement XInclude processing
 *
 * World Wide Web Consortium W3C Last Call Working Draft 16 May 2001
 * http://www.w3.org/TR/2001/WD-xinclude-20010516/
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#define IN_LIBXML
#include "nbsptool/libs/libxml/libxml.h"

#include <string.h>
#include <nbsptool/libs/libxml/xmlmemory.h>
#include <nbsptool/libs/libxml/tree.h>
#include <nbsptool/libs/libxml/parser.h>
#include <nbsptool/libs/libxml/uri.h>
#include <nbsptool/libs/libxml/xpointer.h>
#include <nbsptool/libs/libxml/parserInternals.h>
#include <nbsptool/libs/libxml/xmlerror.h>
#include <nbsptool/libs/libxml/encoding.h>
#include <nbsptool/libs/libxml/globals.h>

#ifdef LIBXML_XINCLUDE_ENABLED
#include <nbsptool/libs/libxml/xinclude.h>


#define XINCLUDE_MAX_DEPTH 40

/* #define DEBUG_XINCLUDE */
#ifdef DEBUG_XINCLUDE
#ifdef LIBXML_DEBUG_ENABLED
#include <nbsptool/libs/libxml/debugXML.h>
#endif
#endif

/************************************************************************
 *                                    *
 *            XInclude contexts handling            *
 *                                    *
 ************************************************************************/

/*
 * An XInclude context
 */
typedef xmlChar *xmlURL;

typedef struct _xmlXIncludeRef xmlXIncludeRef;
typedef xmlXIncludeRef *xmlXIncludeRefPtr;
struct _xmlXIncludeRef {
    xmlChar              *URI; /* the rully resolved resource URL */
    xmlChar         *fragment; /* the fragment in the URI */
    xmlDocPtr          doc; /* the parsed document */
    xmlNodePtr            ref; /* the node making the reference in the source */
    xmlNodePtr            inc; /* the included copy */
    int                   xml; /* xml or txt */
    int                 count; /* how many refs use that specific doc */
    xmlXPathObjectPtr    xptr; /* the xpointer if needed */
};

struct _xmlXIncludeCtxt {
    xmlDocPtr             doc; /* the source document */
    int               incBase; /* the first include for this document */
    int                 incNr; /* number of includes */
    int                incMax; /* size of includes tab */
    xmlXIncludeRefPtr *incTab; /* array of included references */

    int                 txtNr; /* number of unparsed documents */
    int                txtMax; /* size of unparsed documents tab */
    xmlNodePtr        *txtTab; /* array of unparsed text nodes */
    xmlURL         *txturlTab; /* array of unparsed txtuments URLs */

    xmlChar *             url; /* the current URL processed */
    int                 urlNr; /* number of url stacked */
    int                urlMax; /* size of url stack */
    xmlChar *         *urlTab; /* url stack */

    int              nbErrors; /* the number of errors detected */
};

static int
xmlXIncludeDoProcess(xmlXIncludeCtxtPtr ctxt, xmlDocPtr doc, xmlNodePtr tree);


/************************************************************************
 *                                    *
 *             XInclude error handler                *
 *                                    *
 ************************************************************************/

/**
 * xmlXIncludeErrMemory:
 * @extra:  extra informations
 *
 * Handle an out of memory condition
 */
static void
xmlXIncludeErrMemory(xmlXIncludeCtxtPtr ctxt, xmlNodePtr node,
                     const char *extra)
{
    if (ctxt != NULL)
    ctxt->nbErrors++;
    __xmlRaiseError(NULL, NULL, NULL, ctxt, node, XML_FROM_XINCLUDE,
                    XML_ERR_NO_MEMORY, XML_ERR_ERROR, NULL, 0,
            extra, NULL, NULL, 0, 0,
            "Memory allocation failed : %s\n", extra);
}

/**
 * xmlXIncludeErr:
 * @ctxt: the XInclude context
 * @node: the context node
 * @msg:  the error message
 * @extra:  extra informations
 *
 * Handle a resource access error
 */
static void
xmlXIncludeErr(xmlXIncludeCtxtPtr ctxt, xmlNodePtr node, int error,
               const char *msg, const xmlChar *extra)
{
    if (ctxt != NULL)
    ctxt->nbErrors++;
    __xmlRaiseError(NULL, NULL, NULL, ctxt, node, XML_FROM_XINCLUDE,
                    error, XML_ERR_ERROR, NULL, 0,
            (const char *) extra, NULL, NULL, 0, 0,
            msg, (const char *) extra);
}

/**
 * xmlXIncludeFreeRef:
 * @ref: the XInclude reference
 *
 * Free an XInclude reference
 */
static void
xmlXIncludeFreeRef(xmlXIncludeRefPtr ref) {
    if (ref == NULL)
    return;
#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Freeing ref\n");
#endif
    if (ref->doc != NULL) {
#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Freeing doc %s\n", ref->URI);
#endif
    xmlFreeDoc(ref->doc);
    }
    if (ref->URI != NULL)
    xmlFree(ref->URI);
    if (ref->fragment != NULL)
    xmlFree(ref->fragment);
    if (ref->xptr != NULL)
    xmlXPathFreeObject(ref->xptr);
    xmlFree(ref);
}

/**
 * xmlXIncludeNewRef:
 * @ctxt: the XInclude context
 * @URI:  the resource URI
 *
 * Creates a new reference within an XInclude context
 *
 * Returns the new set
 */
static xmlXIncludeRefPtr
xmlXIncludeNewRef(xmlXIncludeCtxtPtr ctxt, const xmlChar *URI,
              xmlNodePtr ref) {
    xmlXIncludeRefPtr ret;

#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "New ref %s\n", URI);
#endif
    ret = (xmlXIncludeRefPtr) xmlMalloc(sizeof(xmlXIncludeRef));
    if (ret == NULL) {
        xmlXIncludeErrMemory(ctxt, ref, "growing XInclude context");
    return(NULL);
    }
    memset(ret, 0, sizeof(xmlXIncludeRef));
    if (URI == NULL)
    ret->URI = NULL;
    else
    ret->URI = xmlStrdup(URI);
    ret->fragment = NULL;
    ret->ref = ref;
    ret->doc = 0;
    ret->count = 0;
    ret->xml = 0;
    ret->inc = NULL;
    if (ctxt->incMax == 0) {
    ctxt->incMax = 4;
        ctxt->incTab = (xmlXIncludeRefPtr *) xmlMalloc(ctxt->incMax *
                          sizeof(ctxt->incTab[0]));
        if (ctxt->incTab == NULL) {
        xmlXIncludeErrMemory(ctxt, ref, "growing XInclude context");
        xmlXIncludeFreeRef(ret);
        return(NULL);
    }
    }
    if (ctxt->incNr >= ctxt->incMax) {
    ctxt->incMax *= 2;
        ctxt->incTab = (xmlXIncludeRefPtr *) xmlRealloc(ctxt->incTab,
                 ctxt->incMax * sizeof(ctxt->incTab[0]));
        if (ctxt->incTab == NULL) {
        xmlXIncludeErrMemory(ctxt, ref, "growing XInclude context");
        xmlXIncludeFreeRef(ret);
        return(NULL);
    }
    }
    ctxt->incTab[ctxt->incNr++] = ret;
    return(ret);
}

/**
 * xmlXIncludeNewContext:
 * @doc:  an XML Document
 *
 * Creates a new XInclude context
 *
 * Returns the new set
 */
xmlXIncludeCtxtPtr
xmlXIncludeNewContext(xmlDocPtr doc) {
    xmlXIncludeCtxtPtr ret;

#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "New context\n");
#endif
    if (doc == NULL)
    return(NULL);
    ret = (xmlXIncludeCtxtPtr) xmlMalloc(sizeof(xmlXIncludeCtxt));
    if (ret == NULL) {
    xmlXIncludeErrMemory(NULL, (xmlNodePtr) doc,
                         "creating XInclude context");
    return(NULL);
    }
    memset(ret, 0, sizeof(xmlXIncludeCtxt));
    ret->doc = doc;
    ret->incNr = 0;
    ret->incBase = 0;
    ret->incMax = 0;
    ret->incTab = NULL;
    ret->nbErrors = 0;
    return(ret);
}

/**
 * xmlXIncludeURLPush:
 * @ctxt:  the parser context
 * @value:  the url
 *
 * Pushes a new url on top of the url stack
 *
 * Returns -1 in case of error, the index in the stack otherwise
 */
static int
xmlXIncludeURLPush(xmlXIncludeCtxtPtr ctxt,
               const xmlChar *value)
{
    if (ctxt->urlNr > XINCLUDE_MAX_DEPTH) {
    xmlXIncludeErr(ctxt, NULL, XML_XINCLUDE_RECURSION,
                   "detected a recursion in %s\n", value);
    return(-1);
    }
    if (ctxt->urlTab == NULL) {
    ctxt->urlMax = 4;
    ctxt->urlNr = 0;
    ctxt->urlTab = (xmlChar * *) xmlMalloc(
                ctxt->urlMax * sizeof(ctxt->urlTab[0]));
        if (ctxt->urlTab == NULL) {
        xmlXIncludeErrMemory(ctxt, NULL, "adding URL");
            return (-1);
        }
    }
    if (ctxt->urlNr >= ctxt->urlMax) {
        ctxt->urlMax *= 2;
        ctxt->urlTab =
            (xmlChar * *) xmlRealloc(ctxt->urlTab,
                                      ctxt->urlMax *
                                      sizeof(ctxt->urlTab[0]));
        if (ctxt->urlTab == NULL) {
        xmlXIncludeErrMemory(ctxt, NULL, "adding URL");
            return (-1);
        }
    }
    ctxt->url = ctxt->urlTab[ctxt->urlNr] = xmlStrdup(value);
    return (ctxt->urlNr++);
}

/**
 * xmlXIncludeURLPop:
 * @ctxt: the parser context
 *
 * Pops the top url from the url stack
 */
static void
xmlXIncludeURLPop(xmlXIncludeCtxtPtr ctxt)
{
    xmlChar * ret;

    if (ctxt->urlNr <= 0)
        return;
    ctxt->urlNr--;
    if (ctxt->urlNr > 0)
        ctxt->url = ctxt->urlTab[ctxt->urlNr - 1];
    else
        ctxt->url = NULL;
    ret = ctxt->urlTab[ctxt->urlNr];
    ctxt->urlTab[ctxt->urlNr] = 0;
    if (ret != NULL)
    xmlFree(ret);
}

/**
 * xmlXIncludeFreeContext:
 * @ctxt: the XInclude context
 *
 * Free an XInclude context
 */
void
xmlXIncludeFreeContext(xmlXIncludeCtxtPtr ctxt) {
    int i;

#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Freeing context\n");
#endif
    if (ctxt == NULL)
    return;
    while (ctxt->urlNr > 0)
    xmlXIncludeURLPop(ctxt);
    if (ctxt->urlTab != NULL)
    xmlFree(ctxt->urlTab);
    for (i = 0;i < ctxt->incNr;i++) {
    if (ctxt->incTab[i] != NULL)
        xmlXIncludeFreeRef(ctxt->incTab[i]);
    }
    for (i = 0;i < ctxt->txtNr;i++) {
    if (ctxt->txturlTab[i] != NULL)
        xmlFree(ctxt->txturlTab[i]);
    }
    if (ctxt->incTab != NULL)
    xmlFree(ctxt->incTab);
    if (ctxt->txtTab != NULL)
    xmlFree(ctxt->txtTab);
    if (ctxt->txturlTab != NULL)
    xmlFree(ctxt->txturlTab);
    xmlFree(ctxt);
}

/**
 * xmlXIncludeParseFile:
 * @ctxt:  the XInclude context
 * @URL:  the URL or file path
 * 
 * parse an document for XInclude
 */
static xmlDocPtr
xmlXIncludeParseFile(xmlXIncludeCtxtPtr ctxt ATTRIBUTE_UNUSED, const char *URL) {
    xmlDocPtr ret;
    xmlParserCtxtPtr pctxt;
    char *directory = NULL;

    xmlInitParser();

    pctxt = xmlCreateFileParserCtxt(URL);
    if (pctxt == NULL) {
    return(NULL);
    }

    if ((pctxt->directory == NULL) && (directory == NULL))
        directory = xmlParserGetDirectory(URL);
    if ((pctxt->directory == NULL) && (directory != NULL))
        pctxt->directory = (char *) xmlStrdup((xmlChar *) directory);

    pctxt->loadsubset = XML_DETECT_IDS;

    xmlParseDocument(pctxt);

    if (pctxt->wellFormed)
        ret = pctxt->myDoc;
    else {
        ret = NULL;
        xmlFreeDoc(pctxt->myDoc);
        pctxt->myDoc = NULL;
    }
    xmlFreeParserCtxt(pctxt);
    
    return(ret);
}

/**
 * xmlXIncludeAddNode:
 * @ctxt:  the XInclude context
 * @cur:  the new node
 * 
 * Add a new node to process to an XInclude context
 */
static int
xmlXIncludeAddNode(xmlXIncludeCtxtPtr ctxt, xmlNodePtr cur) {
    xmlXIncludeRefPtr ref;
    xmlURIPtr uri;
    xmlChar *URL;
    xmlChar *fragment = NULL;
    xmlChar *href;
    xmlChar *parse;
    xmlChar *base;
    xmlChar *URI;
    int xml = 1, i; /* default Issue 64 */
    int local = 0;


    if (ctxt == NULL)
    return(-1);
    if (cur == NULL)
    return(-1);

#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Add node\n");
#endif
    /*
     * read the attributes
     */
    href = xmlGetNsProp(cur, XINCLUDE_NS, XINCLUDE_HREF);
    if (href == NULL) {
    href = xmlGetProp(cur, XINCLUDE_HREF);
    if (href == NULL) {
        xmlXIncludeErr(ctxt, cur, XML_XINCLUDE_NO_HREF,
                       "no href\n", NULL);
        return(-1);
    }
    }
    if (href[0] == '#')
    local = 1;
    parse = xmlGetNsProp(cur, XINCLUDE_NS, XINCLUDE_PARSE);
    if (parse == NULL) {
    parse = xmlGetProp(cur, XINCLUDE_PARSE);
    }
    if (parse != NULL) {
    if (xmlStrEqual(parse, XINCLUDE_PARSE_XML))
        xml = 1;
    else if (xmlStrEqual(parse, XINCLUDE_PARSE_TEXT))
        xml = 0;
    else {
        xmlXIncludeErr(ctxt, cur, XML_XINCLUDE_PARSE_VALUE,
                       "invalid value %s for 'parse'\n", parse);
        if (href != NULL)
        xmlFree(href);
        if (parse != NULL)
        xmlFree(parse);
        return(-1);
    }
    }

    /*
     * compute the URI
     */
    base = xmlNodeGetBase(ctxt->doc, cur);
    if (base == NULL) {
    URI = xmlBuildURI(href, ctxt->doc->URL);
    } else {
    URI = xmlBuildURI(href, base);
    }
    if (URI == NULL) {
    xmlChar *escbase;
    xmlChar *eschref;
    /*
     * Some escaping may be needed
     */
    escbase = xmlURIEscape(base);
    eschref = xmlURIEscape(href);
    URI = xmlBuildURI(eschref, escbase);
    if (escbase != NULL)
        xmlFree(escbase);
    if (eschref != NULL)
        xmlFree(eschref);
    }
    if (parse != NULL)
    xmlFree(parse);
    if (href != NULL)
    xmlFree(href);
    if (base != NULL)
    xmlFree(base);
    if (URI == NULL) {
    xmlXIncludeErr(ctxt, cur, XML_XINCLUDE_HREF_URI,
                   "failed build URL\n", NULL);
    return(-1);
    }

    /*
     * Check the URL and remove any fragment identifier
     */
    uri = xmlParseURI((const char *)URI);
    if (uri == NULL) {
    xmlXIncludeErr(ctxt, cur, XML_XINCLUDE_HREF_URI,
                   "invalid value URI %s\n", URI);
    return(-1);
    }
    if (uri->fragment != NULL) {
    fragment = (xmlChar *) uri->fragment;
    uri->fragment = NULL;
    }
    URL = xmlSaveUri(uri);
    xmlFreeURI(uri);
    xmlFree(URI);
    if (URL == NULL) {
    xmlXIncludeErr(ctxt, cur, XML_XINCLUDE_HREF_URI,
                   "invalid value URI %s\n", URI);
    if (fragment != NULL)
        xmlFree(fragment);
    return(-1);
    }

    /*
     * Check the URL against the stack for recursions
     */
    if (!local) {
    for (i = 0;i < ctxt->urlNr;i++) {
        if (xmlStrEqual(URL, ctxt->urlTab[i])) {
        xmlXIncludeErr(ctxt, cur, XML_XINCLUDE_RECURSION,
                       "detected a recursion in %s\n", URL);
        return(-1);
        }
    }
    }

    ref = xmlXIncludeNewRef(ctxt, URL, cur);
    if (ref == NULL) {
    return(-1);
    }
    ref->fragment = fragment;
    ref->doc = NULL;
    ref->xml = xml;
    ref->count = 1;
    xmlFree(URL);
    return(0);
}

/**
 * xmlXIncludeRecurseDoc:
 * @ctxt:  the XInclude context
 * @doc:  the new document
 * @url:  the associated URL
 * 
 * The XInclude recursive nature is handled at this point.
 */
static void
xmlXIncludeRecurseDoc(xmlXIncludeCtxtPtr ctxt, xmlDocPtr doc,
                  const xmlURL url ATTRIBUTE_UNUSED) {
    xmlXIncludeCtxtPtr newctxt;
    int i;

    /*
     * Avoid recursion in already substitued resources
    for (i = 0;i < ctxt->urlNr;i++) {
    if (xmlStrEqual(doc->URL, ctxt->urlTab[i]))
        return;
    }
     */

#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Recursing in doc %s\n", doc->URL);
#endif
    /*
     * Handle recursion here.
     */

    newctxt = xmlXIncludeNewContext(doc);
    if (newctxt != NULL) {
    /*
     * Copy the existing document set
     */
    newctxt->incMax = ctxt->incMax;
    newctxt->incNr = ctxt->incNr;
        newctxt->incTab = (xmlXIncludeRefPtr *) xmlMalloc(newctxt->incMax *
                                  sizeof(newctxt->incTab[0]));
        if (newctxt->incTab == NULL) {
        xmlXIncludeErrMemory(ctxt, (xmlNodePtr) doc, "processing doc");
        xmlFree(newctxt);
        return;
    }
    /*
     * copy the urlTab
     */
    newctxt->urlMax = ctxt->urlMax;
    newctxt->urlNr = ctxt->urlNr;
    newctxt->urlTab = ctxt->urlTab;

    /*
     * Inherit the documents already in use by others includes
     */
    newctxt->incBase = ctxt->incNr;
    for (i = 0;i < ctxt->incNr;i++) {
        newctxt->incTab[i] = ctxt->incTab[i];
        newctxt->incTab[i]->count++; /* prevent the recursion from
                        freeing it */
    }
    xmlXIncludeDoProcess(newctxt, doc, xmlDocGetRootElement(doc));
    for (i = 0;i < ctxt->incNr;i++) {
        newctxt->incTab[i]->count--;
        newctxt->incTab[i] = NULL;
    }

    /* urlTab may have been reallocated */
    ctxt->urlTab = newctxt->urlTab;
    ctxt->urlMax = newctxt->urlMax;

    newctxt->urlMax = 0;
    newctxt->urlNr = 0;
    newctxt->urlTab = NULL;

    xmlXIncludeFreeContext(newctxt);
    }
#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Done recursing in doc %s\n", url);
#endif
}

/**
 * xmlXIncludeAddTxt:
 * @ctxt:  the XInclude context
 * @txt:  the new text node
 * @url:  the associated URL
 * 
 * Add a new txtument to the list
 */
static void
xmlXIncludeAddTxt(xmlXIncludeCtxtPtr ctxt, xmlNodePtr txt, const xmlURL url) {
#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Adding text %s\n", url);
#endif
    if (ctxt->txtMax == 0) {
    ctxt->txtMax = 4;
        ctxt->txtTab = (xmlNodePtr *) xmlMalloc(ctxt->txtMax *
                                  sizeof(ctxt->txtTab[0]));
        if (ctxt->txtTab == NULL) {
        xmlXIncludeErrMemory(ctxt, NULL, "processing text");
        return;
    }
        ctxt->txturlTab = (xmlURL *) xmlMalloc(ctxt->txtMax *
                                  sizeof(ctxt->txturlTab[0]));
        if (ctxt->txturlTab == NULL) {
        xmlXIncludeErrMemory(ctxt, NULL, "processing text");
        return;
    }
    }
    if (ctxt->txtNr >= ctxt->txtMax) {
    ctxt->txtMax *= 2;
        ctxt->txtTab = (xmlNodePtr *) xmlRealloc(ctxt->txtTab,
                 ctxt->txtMax * sizeof(ctxt->txtTab[0]));
        if (ctxt->txtTab == NULL) {
        xmlXIncludeErrMemory(ctxt, NULL, "processing text");
        return;
    }
        ctxt->txturlTab = (xmlURL *) xmlRealloc(ctxt->txturlTab,
                 ctxt->txtMax * sizeof(ctxt->txturlTab[0]));
        if (ctxt->txturlTab == NULL) {
        xmlXIncludeErrMemory(ctxt, NULL, "processing text");
        return;
    }
    }
    ctxt->txtTab[ctxt->txtNr] = txt;
    ctxt->txturlTab[ctxt->txtNr] = xmlStrdup(url);
    ctxt->txtNr++;
}

/************************************************************************
 *                                    *
 *            Node copy with specific semantic        *
 *                                    *
 ************************************************************************/

/**
 * xmlXIncludeCopyNode:
 * @ctxt:  the XInclude context
 * @target:  the document target
 * @source:  the document source
 * @elem:  the element
 * 
 * Make a copy of the node while preserving the XInclude semantic
 * of the Infoset copy
 */
static xmlNodePtr
xmlXIncludeCopyNode(xmlXIncludeCtxtPtr ctxt, xmlDocPtr target,
                xmlDocPtr source, xmlNodePtr elem) {
    xmlNodePtr result = NULL;

    if ((ctxt == NULL) || (target == NULL) || (source == NULL) ||
    (elem == NULL))
    return(NULL);
    if (elem->type == XML_DTD_NODE)
    return(NULL);
    result = xmlDocCopyNode(elem, target, 1);
    return(result);
}

/**
 * xmlXIncludeCopyNodeList:
 * @ctxt:  the XInclude context
 * @target:  the document target
 * @source:  the document source
 * @elem:  the element list
 * 
 * Make a copy of the node list while preserving the XInclude semantic
 * of the Infoset copy
 */
static xmlNodePtr
xmlXIncludeCopyNodeList(xmlXIncludeCtxtPtr ctxt, xmlDocPtr target,
                    xmlDocPtr source, xmlNodePtr elem) {
    xmlNodePtr cur, res, result = NULL, last = NULL;

    if ((ctxt == NULL) || (target == NULL) || (source == NULL) ||
    (elem == NULL))
    return(NULL);
    cur = elem;
    while (cur != NULL) {
    res = xmlXIncludeCopyNode(ctxt, target, source, cur);
    if (res != NULL) {
        if (result == NULL) {
        result = last = res;
        } else {
        last->next = res;
        res->prev = last;
        last = res;
        }
    }
    cur = cur->next;
    }
    return(result);
}

/**
 * xmlXInclueGetNthChild:
 * @cur:  the node
 * @no:  the child number
 *
 * Returns the @no'th element child of @cur or NULL
 */
static xmlNodePtr
xmlXIncludeGetNthChild(xmlNodePtr cur, int no) {
    int i;
    if (cur == NULL) 
    return(cur);
    cur = cur->children;
    for (i = 0;i <= no;cur = cur->next) {
    if (cur == NULL) 
        return(cur);
    if ((cur->type == XML_ELEMENT_NODE) ||
        (cur->type == XML_DOCUMENT_NODE) ||
        (cur->type == XML_HTML_DOCUMENT_NODE)) {
        i++;
        if (i == no)
        break;
    }
    }
    return(cur);
}

xmlNodePtr xmlXPtrAdvanceNode(xmlNodePtr cur);

/**
 * xmlXIncludeCopyRange:
 * @ctxt:  the XInclude context
 * @target:  the document target
 * @source:  the document source
 * @obj:  the XPointer result from the evaluation.
 *
 * Build a node list tree copy of the XPointer result.
 *
 * Returns an xmlNodePtr list or NULL.
 *         the caller has to free the node tree.
 */
static xmlNodePtr
xmlXIncludeCopyRange(xmlXIncludeCtxtPtr ctxt, xmlDocPtr target,
                    xmlDocPtr source, xmlXPathObjectPtr range) {
    /* pointers to generated nodes */
    xmlNodePtr list = NULL, last = NULL, parent = NULL, tmp;
    /* pointers to traversal nodes */
    xmlNodePtr start, cur, end;
    int index1, index2;

    if ((ctxt == NULL) || (target == NULL) || (source == NULL) ||
    (range == NULL))
    return(NULL);
    if (range->type != XPATH_RANGE)
    return(NULL);
    start = (xmlNodePtr) range->user;

    if (start == NULL)
    return(NULL);
    end = range->user2;
    if (end == NULL)
    return(xmlDocCopyNode(start, target, 1));

    cur = start;
    index1 = range->index;
    index2 = range->index2;
    while (cur != NULL) {
    if (cur == end) {
        if (cur->type == XML_TEXT_NODE) {
        const xmlChar *content = cur->content;
        int len;

        if (content == NULL) {
            tmp = xmlNewTextLen(NULL, 0);
        } else {
            len = index2;
            if ((cur == start) && (index1 > 1)) {
            content += (index1 - 1);
            len -= (index1 - 1);
            index1 = 0;
            } else {
            len = index2;
            }
            tmp = xmlNewTextLen(content, len);
        }
        /* single sub text node selection */
        if (list == NULL)
            return(tmp);
        /* prune and return full set */
        if (last != NULL)
            xmlAddNextSibling(last, tmp);
        else 
            xmlAddChild(parent, tmp);
        return(list);
        } else {
        tmp = xmlDocCopyNode(cur, target, 0);
        if (list == NULL)
            list = tmp;
        else {
            if (last != NULL)
            xmlAddNextSibling(last, tmp);
            else
            xmlAddChild(parent, tmp);
        }
        last = NULL;
        parent = tmp;

        if (index2 > 1) {
            end = xmlXIncludeGetNthChild(cur, index2 - 1);
            index2 = 0;
        }
        if ((cur == start) && (index1 > 1)) {
            cur = xmlXIncludeGetNthChild(cur, index1 - 1);
            index1 = 0;
        } else {
            cur = cur->children;
        }
        /*
         * Now gather the remaining nodes from cur to end
         */
        continue; /* while */
        }
    } else if ((cur == start) &&
           (list == NULL) /* looks superfluous but ... */ ) {
        if ((cur->type == XML_TEXT_NODE) ||
        (cur->type == XML_CDATA_SECTION_NODE)) {
        const xmlChar *content = cur->content;

        if (content == NULL) {
            tmp = xmlNewTextLen(NULL, 0);
        } else {
            if (index1 > 1) {
            content += (index1 - 1);
            }
            tmp = xmlNewText(content);
        }
        last = list = tmp;
        } else {
        if ((cur == start) && (index1 > 1)) {
            tmp = xmlDocCopyNode(cur, target, 0);
            list = tmp;
            parent = tmp;
            last = NULL;
            cur = xmlXIncludeGetNthChild(cur, index1 - 1);
            index1 = 0;
            /*
             * Now gather the remaining nodes from cur to end
             */
            continue; /* while */
        }
        tmp = xmlDocCopyNode(cur, target, 1);
        list = tmp;
        parent = NULL;
        last = tmp;
        }
    } else {
        tmp = NULL;
        switch (cur->type) {
        case XML_DTD_NODE:
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_NODE:
            /* Do not copy DTD informations */
            break;
        case XML_ENTITY_DECL:
            /* handle crossing entities -> stack needed */
            break;
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
            /* don't consider it part of the tree content */
            break;
        case XML_ATTRIBUTE_NODE:
            /* Humm, should not happen ! */
            break;
        default:
            tmp = xmlDocCopyNode(cur, target, 1);
            break;
        }
        if (tmp != NULL) {
        if ((list == NULL) || ((last == NULL) && (parent == NULL)))  {
            return(NULL);
        }
        if (last != NULL)
            xmlAddNextSibling(last, tmp);
        else {
            xmlAddChild(parent, tmp);
            last = tmp;
        }
        }
    }
    /*
     * Skip to next node in document order
     */
    if ((list == NULL) || ((last == NULL) && (parent == NULL)))  {
        return(NULL);
    }
    cur = xmlXPtrAdvanceNode(cur);
    }
    return(list);
}

/**
 * xmlXIncludeBuildNodeList:
 * @ctxt:  the XInclude context
 * @target:  the document target
 * @source:  the document source
 * @obj:  the XPointer result from the evaluation.
 *
 * Build a node list tree copy of the XPointer result.
 * This will drop Attributes and Namespace declarations.
 *
 * Returns an xmlNodePtr list or NULL.
 *         the caller has to free the node tree.
 */
static xmlNodePtr
xmlXIncludeCopyXPointer(xmlXIncludeCtxtPtr ctxt, xmlDocPtr target,
                    xmlDocPtr source, xmlXPathObjectPtr obj) {
    xmlNodePtr list = NULL, last = NULL;
    int i;

    if (source == NULL)
    source = ctxt->doc;
    if ((ctxt == NULL) || (target == NULL) || (source == NULL) ||
    (obj == NULL))
    return(NULL);
    switch (obj->type) {
        case XPATH_NODESET: {
        xmlNodeSetPtr set = obj->nodesetval;
        if (set == NULL)
        return(NULL);
        for (i = 0;i < set->nodeNr;i++) {
        if (set->nodeTab[i] == NULL)
            continue;
        switch (set->nodeTab[i]->type) {
            case XML_TEXT_NODE:
            case XML_CDATA_SECTION_NODE:
            case XML_ELEMENT_NODE:
            case XML_ENTITY_REF_NODE:
            case XML_ENTITY_NODE:
            case XML_PI_NODE:
            case XML_COMMENT_NODE:
            case XML_DOCUMENT_NODE:
            case XML_HTML_DOCUMENT_NODE:
#ifdef LIBXML_DOCB_ENABLED
            case XML_DOCB_DOCUMENT_NODE:
#endif
            case XML_XINCLUDE_END:
            break;
            case XML_XINCLUDE_START: {
                    xmlNodePtr tmp, cur = set->nodeTab[i];

            cur = cur->next;
            while (cur != NULL) {
                switch(cur->type) {
                case XML_TEXT_NODE:
                case XML_CDATA_SECTION_NODE:
                case XML_ELEMENT_NODE:
                case XML_ENTITY_REF_NODE:
                case XML_ENTITY_NODE:
                case XML_PI_NODE:
                case XML_COMMENT_NODE:
                    tmp = xmlXIncludeCopyNode(ctxt, target,
                                  source, cur);
                    if (last == NULL) {
                    list = last = tmp;
                    } else {
                    xmlAddNextSibling(last, tmp);
                    last = tmp;
                    }
                    cur = cur->next;
                    continue;
                default:
                    break;
                }
                break;
            }
            continue;
            }
            case XML_ATTRIBUTE_NODE:
            case XML_NAMESPACE_DECL:
            case XML_DOCUMENT_TYPE_NODE:
            case XML_DOCUMENT_FRAG_NODE:
            case XML_NOTATION_NODE:
            case XML_DTD_NODE:
            case XML_ELEMENT_DECL:
            case XML_ATTRIBUTE_DECL:
            case XML_ENTITY_DECL:
            continue; /* for */
        }
        if (last == NULL)
            list = last = xmlXIncludeCopyNode(ctxt, target, source,
                                          set->nodeTab[i]);
        else {
            xmlAddNextSibling(last,
                xmlXIncludeCopyNode(ctxt, target, source,
                                set->nodeTab[i]));
            if (last->next != NULL)
            last = last->next;
        }
        }
        break;
    }
    case XPATH_LOCATIONSET: {
        xmlLocationSetPtr set = (xmlLocationSetPtr) obj->user;
        if (set == NULL)
        return(NULL);
        for (i = 0;i < set->locNr;i++) {
        if (last == NULL)
            list = last = xmlXIncludeCopyXPointer(ctxt, target, source,
                                              set->locTab[i]);
        else
            xmlAddNextSibling(last,
                xmlXIncludeCopyXPointer(ctxt, target, source,
                                    set->locTab[i]));
        if (last != NULL) {
            while (last->next != NULL)
            last = last->next;
        }
        }
        break;
    }
#ifdef LIBXML_XPTR_ENABLED
    case XPATH_RANGE:
        return(xmlXIncludeCopyRange(ctxt, target, source, obj));
#endif
    case XPATH_POINT:
        /* points are ignored in XInclude */
        break;
    default:
        break;
    }
    return(list);
}
/************************************************************************
 *                                    *
 *            XInclude I/O handling                *
 *                                    *
 ************************************************************************/

typedef struct _xmlXIncludeMergeData xmlXIncludeMergeData;
typedef xmlXIncludeMergeData *xmlXIncludeMergeDataPtr;
struct _xmlXIncludeMergeData {
    xmlDocPtr doc;
    xmlXIncludeCtxtPtr ctxt;
};

/**
 * xmlXIncludeMergeOneEntity:
 * @ent: the entity
 * @doc:  the including doc
 * @nr: the entity name
 *
 * Inplements the merge of one entity
 */
static void
xmlXIncludeMergeEntity(xmlEntityPtr ent, xmlXIncludeMergeDataPtr data,
                   xmlChar *name ATTRIBUTE_UNUSED) {
    xmlEntityPtr ret, prev;
    xmlDocPtr doc;
    xmlXIncludeCtxtPtr ctxt;

    if ((ent == NULL) || (data == NULL))
    return;
    ctxt = data->ctxt;
    doc = data->doc;
    if ((ctxt == NULL) || (doc == NULL))
    return;
    switch (ent->etype) {
        case XML_INTERNAL_PARAMETER_ENTITY:
        case XML_EXTERNAL_PARAMETER_ENTITY:
        case XML_INTERNAL_PREDEFINED_ENTITY:
        return;
        case XML_INTERNAL_GENERAL_ENTITY:
        case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
        case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
        break;
    }
    ret = xmlAddDocEntity(doc, ent->name, ent->etype, ent->ExternalID,
              ent->SystemID, ent->content);
    if (ret != NULL) {
    if (ent->URI != NULL)
        ret->URI = xmlStrdup(ent->URI);
    } else {
    prev = xmlGetDocEntity(doc, ent->name);
    if (prev != NULL) {
        if (ent->etype != prev->etype)
        goto error;
    
        if ((ent->SystemID != NULL) && (prev->SystemID != NULL)) {
        if (!xmlStrEqual(ent->SystemID, prev->SystemID))
            goto error;
        } else if ((ent->ExternalID != NULL) &&
               (prev->ExternalID != NULL)) {
        if (!xmlStrEqual(ent->ExternalID, prev->ExternalID))
            goto error;
        } else if ((ent->content != NULL) && (prev->content != NULL)) {
        if (!xmlStrEqual(ent->content, prev->content))
            goto error;
        } else {
        goto error;
        }

    }
    }
    return;
error:
    switch (ent->etype) {
        case XML_INTERNAL_PARAMETER_ENTITY:
        case XML_EXTERNAL_PARAMETER_ENTITY:
        case XML_INTERNAL_PREDEFINED_ENTITY:
        case XML_INTERNAL_GENERAL_ENTITY:
        case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
        return;
        case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
        break;
    }
    xmlXIncludeErr(ctxt, (xmlNodePtr) ent, XML_XINCLUDE_ENTITY_DEF_MISMATCH,
                   "mismatch in redefinition of entity %s\n",
           ent->name);
}

/**
 * xmlXIncludeMergeEntities:
 * @ctxt: an XInclude context
 * @doc:  the including doc
 * @from:  the included doc
 *
 * Inplements the entity merge
 *
 * Returns 0 if merge succeeded, -1 if some processing failed
 */
static int
xmlXIncludeMergeEntities(xmlXIncludeCtxtPtr ctxt, xmlDocPtr doc,
                     xmlDocPtr from) {
    xmlNodePtr cur;
    xmlDtdPtr target, source;

    if (ctxt == NULL)
    return(-1);

    if ((from == NULL) || (from->intSubset == NULL))
    return(0);

    target = doc->intSubset;
    if (target == NULL) {
    cur = xmlDocGetRootElement(doc);
    if (cur == NULL)
        return(-1);
        target = xmlCreateIntSubset(doc, cur->name, NULL, NULL);
    if (target == NULL)
        return(-1);
    }

    source = from->intSubset;
    if ((source != NULL) && (source->entities != NULL)) {
    xmlXIncludeMergeData data;

    data.ctxt = ctxt;
    data.doc = doc;

    xmlHashScan((xmlHashTablePtr) source->entities,
            (xmlHashScanner) xmlXIncludeMergeEntity, &data);
    }
    source = from->extSubset;
    if ((source != NULL) && (source->entities != NULL)) {
    xmlXIncludeMergeData data;

    data.ctxt = ctxt;
    data.doc = doc;

    /*
     * don't duplicate existing stuff when external subsets are the same
     */
    if ((!xmlStrEqual(target->ExternalID, source->ExternalID)) &&
        (!xmlStrEqual(target->SystemID, source->SystemID))) {
        xmlHashScan((xmlHashTablePtr) source->entities,
            (xmlHashScanner) xmlXIncludeMergeEntity, &data);
    }
    }
    return(0);
}

/**
 * xmlXIncludeLoadDoc:
 * @ctxt:  the XInclude context
 * @url:  the associated URL
 * @nr:  the xinclude node number
 * 
 * Load the document, and store the result in the XInclude context
 *
 * Returns 0 in case of success, -1 in case of failure
 */
static int
xmlXIncludeLoadDoc(xmlXIncludeCtxtPtr ctxt, const xmlChar *url, int nr) {
    xmlDocPtr doc;
    xmlURIPtr uri;
    xmlChar *URL;
    xmlChar *fragment = NULL;
    int i = 0;

#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "Loading doc %s:%d\n", url, nr);
#endif
    /*
     * Check the URL and remove any fragment identifier
     */
    uri = xmlParseURI((const char *)url);
    if (uri == NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                   XML_XINCLUDE_HREF_URI,
               "invalid value URI %s\n", url);
    return(-1);
    }
    if (uri->fragment != NULL) {
    fragment = (xmlChar *) uri->fragment;
    uri->fragment = NULL;
    }
    URL = xmlSaveUri(uri);
    xmlFreeURI(uri);
    if (URL == NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                   XML_XINCLUDE_HREF_URI,
               "invalid value URI %s\n", url);
    if (fragment != NULL)
        xmlFree(fragment);
    return(-1);
    }

    /*
     * Handling of references to the local document are done
     * directly through ctxt->doc.
     */
    if ((URL[0] == 0) || (URL[0] == '#') ||
    ((ctxt->doc != NULL) && (xmlStrEqual(URL, ctxt->doc->URL)))) {
    doc = NULL;
        goto loaded;
    }

    /*
     * Prevent reloading twice the document.
     */
    for (i = 0; i < ctxt->incNr; i++) {
    if ((xmlStrEqual(URL, ctxt->incTab[i]->URI)) &&
        (ctxt->incTab[i]->doc != NULL)) {
        doc = ctxt->incTab[i]->doc;
#ifdef DEBUG_XINCLUDE
        printf("Already loaded %s\n", URL);
#endif
        goto loaded;
    }
    }

    /*
     * Load it.
     */
#ifdef DEBUG_XINCLUDE
    printf("loading %s\n", URL);
#endif
    doc = xmlXIncludeParseFile(ctxt, (const char *)URL);
    if (doc == NULL) {
    xmlFree(URL);
    if (fragment != NULL)
        xmlFree(fragment);
    return(-1);
    }
    ctxt->incTab[nr]->doc = doc;
    for (i = nr + 1; i < ctxt->incNr; i++) {
    if (xmlStrEqual(URL, ctxt->incTab[i]->URI)) {
        ctxt->incTab[nr]->count++;
#ifdef DEBUG_XINCLUDE
        printf("Increasing %s count since reused\n", URL);
#endif
            break;
    }
    }

    /*
     * Make sure we have all entities fixed up
     */
    xmlXIncludeMergeEntities(ctxt, ctxt->doc, doc);

    /*
     * We don't need the DTD anymore, free up space
    if (doc->intSubset != NULL) {
    xmlUnlinkNode((xmlNodePtr) doc->intSubset);
    xmlFreeNode((xmlNodePtr) doc->intSubset);
    doc->intSubset = NULL;
    }
    if (doc->extSubset != NULL) {
    xmlUnlinkNode((xmlNodePtr) doc->extSubset);
    xmlFreeNode((xmlNodePtr) doc->extSubset);
    doc->extSubset = NULL;
    }
     */
    xmlXIncludeRecurseDoc(ctxt, doc, URL);

loaded:
    if (fragment == NULL) {
    /*
     * Add the top children list as the replacement copy.
     */
    if (doc == NULL)
    {
        /* Hopefully a DTD declaration won't be copied from
         * the same document */
        ctxt->incTab[nr]->inc = xmlCopyNodeList(ctxt->doc->children);
    } else {
        ctxt->incTab[nr]->inc = xmlXIncludeCopyNodeList(ctxt, ctxt->doc,
                                               doc, doc->children);
    }
    } 
#ifdef LIBXML_XPTR_ENABLED
    else {
    /*
     * Computes the XPointer expression and make a copy used
     * as the replacement copy.
     */
    xmlXPathObjectPtr xptr;
    xmlXPathContextPtr xptrctxt;
    xmlNodeSetPtr set;

    if (doc == NULL) {
        xptrctxt = xmlXPtrNewContext(ctxt->doc, ctxt->incTab[nr]->ref,
                                 NULL);
    } else {
        xptrctxt = xmlXPtrNewContext(doc, NULL, NULL);
    }
    if (xptrctxt == NULL) {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                       XML_XINCLUDE_XPTR_FAILED,
               "could create XPointer context\n", NULL);
        xmlFree(URL);
        xmlFree(fragment);
        return(-1);
    }
    xptr = xmlXPtrEval(fragment, xptrctxt);
    if (xptr == NULL) {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref,
                       XML_XINCLUDE_XPTR_FAILED,
               "XPointer evaluation failed: #%s\n",
               fragment);
        xmlXPathFreeContext(xptrctxt);
        xmlFree(URL);
        xmlFree(fragment);
        return(-1);
    }
    switch (xptr->type) {
        case XPATH_UNDEFINED:
        case XPATH_BOOLEAN:
        case XPATH_NUMBER:
        case XPATH_STRING:
        case XPATH_POINT:
        case XPATH_USERS:
        case XPATH_XSLT_TREE:
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                       XML_XINCLUDE_XPTR_RESULT,
                   "XPointer is not a range: #%s\n",
                   fragment);
        xmlXPathFreeContext(xptrctxt);
        xmlFree(URL);
        xmlFree(fragment);
        return(-1);
        case XPATH_NODESET:
            if ((xptr->nodesetval == NULL) ||
            (xptr->nodesetval->nodeNr <= 0)) {
            xmlXPathFreeContext(xptrctxt);
            xmlFree(URL);
            xmlFree(fragment);
            return(-1);
        }
        case XPATH_RANGE:
        case XPATH_LOCATIONSET:
        break;
    }
    set = xptr->nodesetval;
    if (set != NULL) {
        for (i = 0;i < set->nodeNr;i++) {
        if (set->nodeTab[i] == NULL)
            continue;
        switch (set->nodeTab[i]->type) {
            case XML_TEXT_NODE:
            case XML_CDATA_SECTION_NODE:
            case XML_ELEMENT_NODE:
            case XML_ENTITY_REF_NODE:
            case XML_ENTITY_NODE:
            case XML_PI_NODE:
            case XML_COMMENT_NODE:
            case XML_DOCUMENT_NODE:
            case XML_HTML_DOCUMENT_NODE:
#ifdef LIBXML_DOCB_ENABLED
            case XML_DOCB_DOCUMENT_NODE:
#endif
            continue;
            case XML_ATTRIBUTE_NODE:
            xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                           XML_XINCLUDE_XPTR_RESULT,
                       "XPointer selects an attribute: #%s\n",
                       fragment);
            set->nodeTab[i] = NULL;
            continue;
            case XML_NAMESPACE_DECL:
            xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                           XML_XINCLUDE_XPTR_RESULT,
                       "XPointer selects a namespace: #%s\n",
                       fragment);
            set->nodeTab[i] = NULL;
            continue;
            case XML_DOCUMENT_TYPE_NODE:
            case XML_DOCUMENT_FRAG_NODE:
            case XML_NOTATION_NODE:
            case XML_DTD_NODE:
            case XML_ELEMENT_DECL:
            case XML_ATTRIBUTE_DECL:
            case XML_ENTITY_DECL:
            case XML_XINCLUDE_START:
            case XML_XINCLUDE_END:
            xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                           XML_XINCLUDE_XPTR_RESULT,
                   "XPointer selects unexpected nodes: #%s\n",
                       fragment);
            set->nodeTab[i] = NULL;
            set->nodeTab[i] = NULL;
            continue; /* for */
        }
        }
    }
    if (doc == NULL) {
        ctxt->incTab[nr]->xptr = xptr;
        ctxt->incTab[nr]->inc = NULL;
    } else {
        ctxt->incTab[nr]->inc =
        xmlXIncludeCopyXPointer(ctxt, ctxt->doc, doc, xptr);
        xmlXPathFreeObject(xptr);
    }
    xmlXPathFreeContext(xptrctxt);
    xmlFree(fragment);
    }
#endif

    /*
     * Do the xml:base fixup if needed
     */
    if ((doc != NULL) && (URL != NULL) && (xmlStrchr(URL, (xmlChar) '/'))) {
    xmlNodePtr node;

    node = ctxt->incTab[nr]->inc;
    while (node != NULL) {
        if (node->type == XML_ELEMENT_NODE)
        xmlNodeSetBase(node, URL);
        node = node->next;
    }
    }
    if ((nr < ctxt->incNr) && (ctxt->incTab[nr]->doc != NULL) &&
    (ctxt->incTab[nr]->count <= 1)) {
#ifdef DEBUG_XINCLUDE
        printf("freeing %s\n", ctxt->incTab[nr]->doc->URL);
#endif
    xmlFreeDoc(ctxt->incTab[nr]->doc);
    ctxt->incTab[nr]->doc = NULL;
    }
    xmlFree(URL);
    return(0);
}

/**
 * xmlXIncludeLoadTxt:
 * @ctxt:  the XInclude context
 * @url:  the associated URL
 * @nr:  the xinclude node number
 * 
 * Load the content, and store the result in the XInclude context
 *
 * Returns 0 in case of success, -1 in case of failure
 */
static int
xmlXIncludeLoadTxt(xmlXIncludeCtxtPtr ctxt, const xmlChar *url, int nr) {
    xmlParserInputBufferPtr buf;
    xmlNodePtr node;
    xmlURIPtr uri;
    xmlChar *URL;
    int i;
    xmlChar *encoding = NULL;
    xmlCharEncoding enc = (xmlCharEncoding) 0;

    /*
     * Check the URL and remove any fragment identifier
     */
    uri = xmlParseURI((const char *)url);
    if (uri == NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, XML_XINCLUDE_HREF_URI,
                   "invalid value URI %s\n", url);
    return(-1);
    }
    if (uri->fragment != NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, XML_XINCLUDE_TEXT_FRAGMENT,
                   "fragment identifier forbidden for text: %s\n",
               (const xmlChar *) uri->fragment);
    xmlFreeURI(uri);
    return(-1);
    }
    URL = xmlSaveUri(uri);
    xmlFreeURI(uri);
    if (URL == NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, XML_XINCLUDE_HREF_URI,
                   "invalid value URI %s\n", url);
    return(-1);
    }

    /*
     * Handling of references to the local document are done
     * directly through ctxt->doc.
     */
    if (URL[0] == 0) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                   XML_XINCLUDE_TEXT_DOCUMENT,
               "text serialization of document not available\n", NULL);
    xmlFree(URL);
    return(-1);
    }

    /*
     * Prevent reloading twice the document.
     */
    for (i = 0; i < ctxt->txtNr; i++) {
    if (xmlStrEqual(URL, ctxt->txturlTab[i])) {
        node = xmlCopyNode(ctxt->txtTab[i], 1);
        goto loaded;
    }
    }
    /*
     * Try to get the encoding if available
     */
    if ((ctxt->incTab[nr] != NULL) && (ctxt->incTab[nr]->ref != NULL)) {
    encoding = xmlGetProp(ctxt->incTab[nr]->ref, XINCLUDE_PARSE_ENCODING);
    }
    if (encoding != NULL) {
    /*
     * TODO: we should not have to remap to the xmlCharEncoding
     *       predefined set, a better interface than
     *       xmlParserInputBufferCreateFilename should allow any
     *       encoding supported by iconv
     */
        enc = xmlParseCharEncoding((const char *) encoding);
    if (enc == XML_CHAR_ENCODING_ERROR) {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref,
                       XML_XINCLUDE_UNKNOWN_ENCODING,
               "encoding %s not supported\n", encoding);
        xmlFree(encoding);
        xmlFree(URL);
        return(-1);
    }
    xmlFree(encoding);
    }

    /*
     * Load it.
     */
    buf = xmlParserInputBufferCreateFilename((const char *)URL, enc);
    if (buf == NULL) {
    xmlFree(URL);
    return(-1);
    }
    node = xmlNewText(NULL);

    /*
     * Scan all chars from the resource and add the to the node
     */
    while (xmlParserInputBufferRead(buf, 128) > 0) {
    int len;
    const xmlChar *content;

    content = xmlBufferContent(buf->buffer);
    len = xmlBufferLength(buf->buffer);
    for (i = 0;i < len;) {
        int cur;
        int l;

        cur = xmlStringCurrentChar(NULL, &content[i], &l);
        if (!IS_CHAR(cur)) {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref,
                       XML_XINCLUDE_INVALID_CHAR,
                   "%s contains invalid char\n", URL);
        } else {
        xmlNodeAddContentLen(node, &content[i], l);
        }
        i += l;
    }
    xmlBufferShrink(buf->buffer, len);
    }
    xmlFreeParserInputBuffer(buf);
    xmlXIncludeAddTxt(ctxt, node, URL);

loaded:
    /*
     * Add the element as the replacement copy.
     */
    ctxt->incTab[nr]->inc = node;
    xmlFree(URL);
    return(0);
}

/**
 * xmlXIncludeLoadFallback:
 * @ctxt:  the XInclude context
 * @fallback:  the fallback node
 * @nr:  the xinclude node number
 * 
 * Load the content of the fallback node, and store the result
 * in the XInclude context
 *
 * Returns 0 in case of success, -1 in case of failure
 */
static int
xmlXIncludeLoadFallback(xmlXIncludeCtxtPtr ctxt, xmlNodePtr fallback, int nr) {
    if ((fallback == NULL) || (ctxt == NULL))
    return(-1);

    ctxt->incTab[nr]->inc = xmlCopyNodeList(fallback->children);
    return(0);
}

/************************************************************************
 *                                    *
 *            XInclude Processing                *
 *                                    *
 ************************************************************************/

/**
 * xmlXIncludePreProcessNode:
 * @ctxt: an XInclude context
 * @node: an XInclude node
 *
 * Implement the XInclude preprocessing, currently just adding the element
 * for further processing.
 *
 * Returns the result list or NULL in case of error
 */
static xmlNodePtr
xmlXIncludePreProcessNode(xmlXIncludeCtxtPtr ctxt, xmlNodePtr node) {
    xmlXIncludeAddNode(ctxt, node);
    return(0);
}

/**
 * xmlXIncludeLoadNode:
 * @ctxt: an XInclude context
 * @nr: the node number
 *
 * Find and load the infoset replacement for the given node.
 *
 * Returns 0 if substitution succeeded, -1 if some processing failed
 */
static int
xmlXIncludeLoadNode(xmlXIncludeCtxtPtr ctxt, int nr) {
    xmlNodePtr cur;
    xmlChar *href;
    xmlChar *parse;
    xmlChar *base;
    xmlChar *URI;
    int xml = 1; /* default Issue 64 */
    int ret;

    if (ctxt == NULL)
    return(-1);
    if ((nr < 0) || (nr >= ctxt->incNr))
    return(-1);
    cur = ctxt->incTab[nr]->ref;
    if (cur == NULL)
    return(-1);

    /*
     * read the attributes
     */
    href = xmlGetNsProp(cur, XINCLUDE_NS, XINCLUDE_HREF);
    if (href == NULL) {
    href = xmlGetProp(cur, XINCLUDE_HREF);
    if (href == NULL) {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                       XML_XINCLUDE_NO_HREF, "no href\n", NULL);
        return(-1);
    }
    }
    parse = xmlGetNsProp(cur, XINCLUDE_NS, XINCLUDE_PARSE);
    if (parse == NULL) {
    parse = xmlGetProp(cur, XINCLUDE_PARSE);
    }
    if (parse != NULL) {
    if (xmlStrEqual(parse, XINCLUDE_PARSE_XML))
        xml = 1;
    else if (xmlStrEqual(parse, XINCLUDE_PARSE_TEXT))
        xml = 0;
    else {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref,
                       XML_XINCLUDE_PARSE_VALUE,
               "invalid value %s for 'parse'\n", parse);
        if (href != NULL)
        xmlFree(href);
        if (parse != NULL)
        xmlFree(parse);
        return(-1);
    }
    }

    /*
     * compute the URI
     */
    base = xmlNodeGetBase(ctxt->doc, cur);
    if (base == NULL) {
    URI = xmlBuildURI(href, ctxt->doc->URL);
    } else {
    URI = xmlBuildURI(href, base);
    }
    if (URI == NULL) {
    xmlChar *escbase;
    xmlChar *eschref;
    /*
     * Some escaping may be needed
     */
    escbase = xmlURIEscape(base);
    eschref = xmlURIEscape(href);
    URI = xmlBuildURI(eschref, escbase);
    if (escbase != NULL)
        xmlFree(escbase);
    if (eschref != NULL)
        xmlFree(eschref);
    }
    if (URI == NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                   XML_XINCLUDE_HREF_URI, "failed build URL\n", NULL);
    if (parse != NULL)
        xmlFree(parse);
    if (href != NULL)
        xmlFree(href);
    if (base != NULL)
        xmlFree(base);
    return(-1);
    }
#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "parse: %s\n",
        xml ? "xml": "text");
    xmlGenericError(xmlGenericErrorContext, "URI: %s\n", URI);
#endif

    /*
     * Cleanup
     */
    if (xml) {
    ret = xmlXIncludeLoadDoc(ctxt, URI, nr);
    /* xmlXIncludeGetFragment(ctxt, cur, URI); */
    } else {
    ret = xmlXIncludeLoadTxt(ctxt, URI, nr);
    }
    if (ret < 0) {
    xmlNodePtr children;

    /*
     * Time to try a fallback if availble
     */
#ifdef DEBUG_XINCLUDE
    xmlGenericError(xmlGenericErrorContext, "error looking for fallback\n");
#endif
    children = cur->children;
    while (children != NULL) {
        if ((children->type == XML_ELEMENT_NODE) &&
        (children->ns != NULL) &&
        (xmlStrEqual(children->name, XINCLUDE_FALLBACK)) &&
        (xmlStrEqual(children->ns->href, XINCLUDE_NS))) {
        ret = xmlXIncludeLoadFallback(ctxt, children, nr);
        if (ret == 0)
            break;
        }
        children = children->next;
    }
    }
    if (ret < 0) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                   XML_XINCLUDE_NO_FALLBACK,
               "could not load %s, and no fallback was found\n",
               URI);
    }

    /*
     * Cleanup
     */
    if (URI != NULL)
    xmlFree(URI);
    if (parse != NULL)
    xmlFree(parse);
    if (href != NULL)
    xmlFree(href);
    if (base != NULL)
    xmlFree(base);
    return(0);
}

/**
 * xmlXIncludeIncludeNode:
 * @ctxt: an XInclude context
 * @nr: the node number
 *
 * Inplement the infoset replacement for the given node
 *
 * Returns 0 if substitution succeeded, -1 if some processing failed
 */
static int
xmlXIncludeIncludeNode(xmlXIncludeCtxtPtr ctxt, int nr) {
    xmlNodePtr cur, end, list, tmp;

    if (ctxt == NULL)
    return(-1);
    if ((nr < 0) || (nr >= ctxt->incNr))
    return(-1);
    cur = ctxt->incTab[nr]->ref;
    if (cur == NULL)
    return(-1);

    /*
     * If we stored an XPointer a late computation may be needed
     */
    if ((ctxt->incTab[nr]->inc == NULL) &&
    (ctxt->incTab[nr]->xptr != NULL)) {
    ctxt->incTab[nr]->inc =
        xmlXIncludeCopyXPointer(ctxt, ctxt->doc, ctxt->doc,
                            ctxt->incTab[nr]->xptr);
    xmlXPathFreeObject(ctxt->incTab[nr]->xptr);
    ctxt->incTab[nr]->xptr = NULL;
    }
    list = ctxt->incTab[nr]->inc;
    ctxt->incTab[nr]->inc = NULL;

    /*
     * Check against the risk of generating a multi-rooted document
     */
    if ((cur->parent != NULL) &&
    (cur->parent->type != XML_ELEMENT_NODE)) {
    int nb_elem = 0;

    tmp = list;
    while (tmp != NULL) {
        if (tmp->type == XML_ELEMENT_NODE)
        nb_elem++;
        tmp = tmp->next;
    }
    if (nb_elem > 1) {
        xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, 
                       XML_XINCLUDE_MULTIPLE_ROOT,
               "XInclude error: would result in multiple root nodes\n",
               NULL);
        return(-1);
    }
    }

    /*
     * Change the current node as an XInclude start one, and add an
     * entity end one
     */
    cur->type = XML_XINCLUDE_START;
    end = xmlNewNode(cur->ns, cur->name);
    if (end == NULL) {
    xmlXIncludeErr(ctxt, ctxt->incTab[nr]->ref, XML_XINCLUDE_BUILD_FAILED,
                   "failed to build node\n", NULL);
    return(-1);
    }
    end->type = XML_XINCLUDE_END;
    xmlAddNextSibling(cur, end);

    /*
     * Add the list of nodes
     */
    while (list != NULL) {
    cur = list;
    list = list->next;

        xmlAddPrevSibling(end, cur);
    }

    
    return(0);
}

/**
 * xmlXIncludeTestNode:
 * @ctxt: the XInclude processing context
 * @node: an XInclude node
 *
 * test if the node is an XInclude node
 *
 * Returns 1 true, 0 otherwise
 */
static int
xmlXIncludeTestNode(xmlXIncludeCtxtPtr ctxt, xmlNodePtr node) {
    if (node == NULL)
    return(0);
    if (node->type != XML_ELEMENT_NODE)
    return(0);
    if (node->ns == NULL)
    return(0);
    if (xmlStrEqual(node->ns->href, XINCLUDE_NS)) {
    if (xmlStrEqual(node->name, XINCLUDE_NODE)) {
        xmlNodePtr child = node->children;
        int nb_fallback = 0;

        while (child != NULL) {
        if ((child->type == XML_ELEMENT_NODE) &&
            (child->ns != NULL) &&
            (xmlStrEqual(child->ns->href, XINCLUDE_NS))) {
            if (xmlStrEqual(child->name, XINCLUDE_NODE)) {
            xmlXIncludeErr(ctxt, node,
                           XML_XINCLUDE_INCLUDE_IN_INCLUDE,
                       "%s has an 'include' child\n",
                       XINCLUDE_NODE);
            return(0);
            }
            if (xmlStrEqual(child->name, XINCLUDE_FALLBACK)) {
            nb_fallback++;
            }
        }
        child = child->next;
        }
        if (nb_fallback > 1) {
        xmlXIncludeErr(ctxt, node, XML_XINCLUDE_FALLBACKS_IN_INCLUDE,
                   "%s has multiple fallback children\n",
                       XINCLUDE_NODE);
        return(0);
        }
        return(1);
    }
    if (xmlStrEqual(node->name, XINCLUDE_FALLBACK)) {
        if ((node->parent == NULL) ||
        (node->parent->type != XML_ELEMENT_NODE) ||
        (node->parent->ns == NULL) ||
        (!xmlStrEqual(node->parent->ns->href, XINCLUDE_NS)) ||
        (!xmlStrEqual(node->parent->name, XINCLUDE_NODE))) {
        xmlXIncludeErr(ctxt, node,
                       XML_XINCLUDE_FALLBACK_NOT_IN_INCLUDE,
                   "%s is not the child of an 'include'\n",
                   XINCLUDE_FALLBACK);
        }
    }
    }
    return(0);
}

/**
 * xmlXIncludeDoProcess:
 * @ctxt: the XInclude processing context
 * @doc: an XML document
 * @tree: the top of the tree to process
 *
 * Implement the XInclude substitution on the XML document @doc
 *
 * Returns 0 if no substitution were done, -1 if some processing failed
 *    or the number of substitutions done.
 */
static int
xmlXIncludeDoProcess(xmlXIncludeCtxtPtr ctxt, xmlDocPtr doc, xmlNodePtr tree) {
    xmlNodePtr cur;
    int ret = 0;
    int i;

    if ((doc == NULL) || (tree == NULL))
    return(-1);
    if (ctxt == NULL)
    return(-1);

    if (doc->URL != NULL) {
    ret = xmlXIncludeURLPush(ctxt, doc->URL);
    if (ret < 0)
        return(-1);
    }

    /*
     * First phase: lookup the elements in the document
     */
    cur = tree;
    if (xmlXIncludeTestNode(ctxt, cur) == 1)
    xmlXIncludePreProcessNode(ctxt, cur);
    while (cur != NULL) {
    /* TODO: need to work on entities -> stack */
    if ((cur->children != NULL) &&
        (cur->children->type != XML_ENTITY_DECL) &&
        (cur->children->type != XML_XINCLUDE_START) &&
        (cur->children->type != XML_XINCLUDE_END)) {
        cur = cur->children;
        if (xmlXIncludeTestNode(ctxt, cur))
        xmlXIncludePreProcessNode(ctxt, cur);
    } else if (cur->next != NULL) {
        cur = cur->next;
        if (xmlXIncludeTestNode(ctxt, cur))
        xmlXIncludePreProcessNode(ctxt, cur);
    } else {
        do {
        cur = cur->parent;
        if (cur == NULL) break; /* do */
        if (cur->next != NULL) {
            cur = cur->next;
            if (xmlXIncludeTestNode(ctxt, cur))
            xmlXIncludePreProcessNode(ctxt, cur);
            break; /* do */
        }
        } while (cur != NULL);
    }
    }

    /*
     * Second Phase : collect the infosets fragments
     */
    for (i = ctxt->incBase;i < ctxt->incNr; i++) {
        xmlXIncludeLoadNode(ctxt, i);
    ret++;
    }

    /*
     * Third phase: extend the original document infoset.
     */
    if (ctxt->nbErrors == 0) {
    for (i = ctxt->incBase;i < ctxt->incNr; i++) {
        xmlXIncludeIncludeNode(ctxt, i);
    }
    }

    if (doc->URL != NULL)
    xmlXIncludeURLPop(ctxt);
    return(ret);
}

/**
 * xmlXIncludeProcess:
 * @doc: an XML document
 *
 * Implement the XInclude substitution on the XML document @doc
 *
 * Returns 0 if no substitution were done, -1 if some processing failed
 *    or the number of substitutions done.
 */
int
xmlXIncludeProcess(xmlDocPtr doc) {
    xmlXIncludeCtxtPtr ctxt;
    xmlNodePtr tree;
    int ret = 0;

    if (doc == NULL)
    return(-1);
    tree = xmlDocGetRootElement(doc);
    if (tree == NULL)
    return(-1);
    ctxt = xmlXIncludeNewContext(doc);
    if (ctxt == NULL)
    return(-1);
    ret = xmlXIncludeDoProcess(ctxt, doc, tree);
    if ((ret >= 0) && (ctxt->nbErrors > 0))
    ret = -1;

    xmlXIncludeFreeContext(ctxt);
    return(ret);
}

/**
 * xmlXIncludeProcessTree:
 * @tree: a node in an XML document
 *
 * Implement the XInclude substitution for the given subtree
 *
 * Returns 0 if no substitution were done, -1 if some processing failed
 *    or the number of substitutions done.
 */
int
xmlXIncludeProcessTree(xmlNodePtr tree) {
    xmlXIncludeCtxtPtr ctxt;
    int ret = 0;

    if ((tree == NULL) || (tree->doc == NULL))
    return(-1);
    ctxt = xmlXIncludeNewContext(tree->doc);
    if (ctxt == NULL)
    return(-1);
    ret = xmlXIncludeDoProcess(ctxt, tree->doc, tree);
    if ((ret >= 0) && (ctxt->nbErrors > 0))
    ret = -1;

    xmlXIncludeFreeContext(ctxt);
    return(ret);
}

/**
 * xmlXIncludeProcessNode:
 * @ctxt: an existing XInclude context
 * @node: a node in an XML document
 *
 * Implement the XInclude substitution for the given subtree reusing
 * the informations and data coming from the given context.
 *
 * Returns 0 if no substitution were done, -1 if some processing failed
 *    or the number of substitutions done.
 */
int
xmlXIncludeProcessNode(xmlXIncludeCtxtPtr ctxt, xmlNodePtr node) {
    int ret = 0;

    if ((node == NULL) || (node->doc == NULL) || (ctxt == NULL))
    return(-1);
    ret = xmlXIncludeDoProcess(ctxt, node->doc, node);
    if ((ret >= 0) && (ctxt->nbErrors > 0))
    ret = -1;
    return(ret);
}

#else /* !LIBXML_XINCLUDE_ENABLED */
#endif
