/*
 *  Based on movescu.cc (Revision: 1.51, Date: 2004/04/06 18:11:24)
 */

/*
 *  Original copyright header:
 *
 *  Copyright (C) 1994-2004, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  dcmnet
 *  Author:  Andrew Hewett
 *
 *  Purpose: Query/Retrieve Service Class User (C-MOVE operation)
 */

#include "dcmtkmovescu.h"
#include "voreendcmtk.h"

#include "tgt/logmanager.h"

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#define INCLUDE_CERRNO
#include <dcmtk/ofstd/ofstdinc.h>

#ifdef HAVE_GUSI_H
#include <GUSI.h>
#endif

#include <dcmtk/dcmnet/dicom.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#if defined(VRN_DCMTK_VERSION_354)
#include <dcmtk/dcmdata/dcdebug.h>
#endif
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmqrdb/dcmqrsrv.h>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/cmdlnarg.h>
#include <dcmtk/ofstd/ofconapp.h>
#include <dcmtk/dcmdata/dcuid.h>    /* for dcmtk version name */
#include <dcmtk/ofstd/ofstd.h>
#include <dcmtk/dcmdata/dcdicent.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/dcasccfg.h>      /* for class DcmAssociationConfiguration */
#include <dcmtk/dcmnet/dcasccff.h>      /* for class DcmAssociationConfigurationFile */

#ifdef WITH_ZLIB
#include <zlib.h>     /* for zlibVersion() */
#endif

#ifdef WITH_OPENSSL
#include <dcmtk/dcmtls/tlstrans.h>
#include <dcmtk/dcmtls/tlslayer.h>
#endif

#ifdef WIN32
#include <WinSock2.h>
#endif

// Prevent warnings because of redefined symbols
#undef PACKAGE_NAME
#undef PACKAGE_BUGREPORT
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

using std::string;
using std::vector;
using std::endl;

std::string voreen::DcmtkMoveSCU::ourtitle_ = "";
int voreen::DcmtkMoveSCU::retrievePort_ = -1;
std::string voreen::DcmtkMoveSCU::peer_ = "";
int voreen::DcmtkMoveSCU::port_ = -1;
int voreen::DcmtkMoveSCU::filesDone_ = 0;
voreen::DcmtkSecurityOptions voreen::DcmtkMoveSCU::security_ = voreen::DcmtkSecurityOptions();

// Start anonymous namespace to encapsulate imported code
namespace {

std::string loggerCat_("voreen.io.DicomMoveSCU");

vector<string>* filelist; // Downloaded files are added here

typedef struct {
    const char *findSyntax;
    const char *moveSyntax;
} QuerySyntax;

typedef struct {
    T_ASC_Association *assoc;
    T_ASC_PresentationContextID presId;
} MyCallbackInfo;

#if !defined(VRN_DCMTK_VERSION_360)
OFCmdUnsignedInt  opt_sleepAfter = 0;
OFCmdUnsignedInt  opt_sleepDuring = 0;
OFBool            opt_useMetaheader = OFTrue;
//E_TransferSyntax  opt_networkTransferSyntax = EXS_Unknown;
E_TransferSyntax  opt_writeTransferSyntax = EXS_Unknown;
E_GrpLenEncoding  opt_groupLength = EGL_recalcGL;
E_EncodingType    opt_sequenceType = EET_ExplicitLength;
E_PaddingEncoding opt_paddingType = EPD_withoutPadding;
OFCmdUnsignedInt  opt_filepad = 0;
OFCmdUnsignedInt  opt_itempad = 0;
OFBool            opt_bitPreserving = OFFalse;
OFBool            opt_ignore = OFFalse;
OFBool            opt_correctUIDPadding = OFFalse;
#endif
OFCmdUnsignedInt  opt_maxPDU = ASC_DEFAULTMAXPDU;
OFBool            opt_abortDuringStore = OFFalse;
OFBool            opt_abortAfterStore = OFFalse;
OFBool            opt_verbose = OFFalse;
OFBool            opt_debug = OFFalse;
OFCmdUnsignedInt  opt_repeatCount = 1;
OFCmdUnsignedInt  opt_retrievePort = 104;
E_TransferSyntax  opt_in_networkTransferSyntax = EXS_Unknown;
E_TransferSyntax  opt_out_networkTransferSyntax = EXS_Unknown;
OFBool            opt_abortAssociation = OFFalse;
const char *      opt_moveDestination = NULL;
OFCmdSignedInt    opt_cancelAfterNResponses = -1;
voreen::DcmtkMoveSCU::QueryModel        opt_queryModel = voreen::DcmtkMoveSCU::QMPatientRoot;
string            filePath = "";

OFBool            opt_secureConnection = OFFalse;
DcmAssociationConfiguration* asccfg = 0;
string opt_configFile;
string opt_profileName = "Default";

static T_ASC_Network *net = NULL; /* the global DICOM network */
static DcmDataset *overrideKeys = NULL;
static QuerySyntax querySyntax[3] = {
    { UID_FINDPatientRootQueryRetrieveInformationModel,
      UID_MOVEPatientRootQueryRetrieveInformationModel },
    { UID_FINDStudyRootQueryRetrieveInformationModel,
      UID_MOVEStudyRootQueryRetrieveInformationModel }
#if defined(VRN_DCMTK_VERSION_354)
    ,{ UID_FINDPatientStudyOnlyQueryRetrieveInformationModel,
      UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
#endif
};


static void
errmsg(const char *msg,...)
{
    va_list args;

    fprintf(stderr, "%s: ", "movescu/mobil");
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#if defined(VRN_DCMTK_VERSION_354)
static void
addOverrideKey(/*OFConsoleApplication& app,*/ const char* s)
{
    unsigned int g = 0xffff;
    unsigned int e = 0xffff;
    int n = 0;
    char val[1024];
    OFString msg;
    char msg2[200];

    val[0] = '\0';
    n = sscanf(s, "%x,%x=%s", &g, &e, val);

    if (n < 2) {
      msg = "bad key format: ";
      msg += s;
      errmsg(msg.c_str());
    }

    const char* spos = s;
    char ccc;
    do
    {
      ccc = *spos;
      if (ccc == '=') break;
      if (ccc == 0) { spos = NULL; break; }
      spos++;
    } while(1);

    if (spos && *(spos+1)) {
        strcpy(val, spos+1);
    }

    DcmTag tag(g,e);
    if (tag.error() != EC_Normal) {
        sprintf(msg2, "unknown tag: (%04x,%04x)", g, e);
        errmsg(msg2);
    }
    DcmElement *elem = newDicomElement(tag);
    if (elem == NULL) {
        sprintf(msg2, "cannot create element for tag: (%04x,%04x)", g, e);
        errmsg(msg2);
    }
    if (strlen(val) > 0) {
        elem->putString(val);
        if (elem->error() != EC_Normal)
        {
            sprintf(msg2, "cannot put tag value: (%04x,%04x)=\"", g, e);
            msg = msg2;
            msg += val;
            msg += "\"";
            errmsg(msg.c_str());
        }
    }

    if (overrideKeys == NULL) overrideKeys = new DcmDataset;
    overrideKeys->insert(elem, OFTrue);
    if (overrideKeys->error() != EC_Normal) {
        sprintf(msg2, "cannot insert tag: (%04x,%04x)", g, e);
        errmsg(msg2);
    }
}
#else
static void
addOverrideKey(/*OFConsoleApplication& app,*/ const char* /*s*/) {
    LWARNINGC("dicommovescu.cpp",
        "addOverrideKey not supported for Dcmtk version " << OFFIS_DCMTK_VERSION_NUMBER);
}
#endif

static OFCondition cmove(T_ASC_Association *assoc, const char *fname);

static OFCondition
addPresentationContext(T_ASC_Parameters *params,
                        T_ASC_PresentationContextID pid,
                        const char* abstractSyntax);

static OFCondition
addPresentationContext(T_ASC_Parameters *params,
                        T_ASC_PresentationContextID pid,
                        const char* abstractSyntax)
{
    /*
    ** We prefer to use Explicitly encoded transfer syntaxes.
    ** If we are running on a Little Endian machine we prefer
    ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
    ** Some SCP implementations will just select the first transfer
    ** syntax they support (this is not part of the standard) so
    ** organise the proposed transfer syntaxes to take advantage
    ** of such behaviour.
    **
    ** The presentation contexts proposed here are only used for
    ** C-FIND and C-MOVE, so there is no need to support compressed
    ** transmission.
    */

    const char* transferSyntaxes[] = { NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    switch (opt_out_networkTransferSyntax) {
    case EXS_LittleEndianImplicit:
        /* we only support Little Endian Implicit */
        transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit */
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit */
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    default:
        /* We prefer explicit transfer syntaxes.
         * If we are running on a Little Endian machine we prefer
         * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
         */
        if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
        {
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        } else {
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    }

    return ASC_addPresentationContext(
        params, pid, abstractSyntax,
        transferSyntaxes, numTransferSyntaxes);
}

static OFCondition
acceptSubAssoc(T_ASC_Network * aNet, T_ASC_Association ** assoc)
{
    const char* knownAbstractSyntaxes[] = {
        UID_VerificationSOPClass
    };
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    int numTransferSyntaxes;

    OFCondition cond = ASC_receiveAssociation(aNet, assoc, opt_maxPDU, NULL, NULL, opt_secureConnection);
    if (cond.good())
    {
      switch (opt_in_networkTransferSyntax)
      {
        case EXS_LittleEndianImplicit:
          /* we only support Little Endian Implicit */
          transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 1;
          break;
        case EXS_LittleEndianExplicit:
          /* we prefer Little Endian Explicit */
          transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 3;
          break;
        case EXS_BigEndianExplicit:
          /* we prefer Big Endian Explicit */
          transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 3;
          break;
        case EXS_JPEGProcess14SV1TransferSyntax:
          /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
          transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEGProcess1TransferSyntax:
          /* we prefer JPEGBaseline (default lossy for 8 bit images) */
          transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEGProcess2_4TransferSyntax:
          /* we prefer JPEGExtended (default lossy for 12 bit images) */
          transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_RLELossless:
          /* we prefer RLE Lossless */
          transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        default:
          /* We prefer explicit transfer syntaxes.
           * If we are running on a Little Endian machine we prefer
           * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
           */
          if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
          {
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
          } else {
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          }
          transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 3;
          break;

        }

      if (!opt_profileName.empty() && !opt_configFile.empty())
      {
          OFString sprofile;
          /* perform name mangling for config file key */
          const char *c = opt_profileName.c_str();
          while (*c)
          {
              if (! isspace(*c)) sprofile += OFstatic_cast(char, toupper(*c));
              ++c;
          }

          /* set presentation contexts as defined in config file */
          cond = asccfg->evaluateAssociationParameters(sprofile.c_str(), **assoc);
          if (cond.bad())
          {
              if (opt_verbose) DimseCondition::dump(cond);
          }
      }
      else
      {
          /* accept the Verification SOP Class if presented */
          cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
              (*assoc)->params,
              knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
              transferSyntaxes, numTransferSyntaxes);

          if (cond.good())
          {
              /* the array of Storage SOP Class UIDs comes from dcuid.h */
              cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                  (*assoc)->params,
                  dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
                  transferSyntaxes, numTransferSyntaxes);
          }
      }
    }
    if (cond.good()) cond = ASC_acknowledgeAssociation(*assoc);
    if (cond.bad()) {
        ASC_dropAssociation(*assoc);
        ASC_destroyAssociation(assoc);
    }
    return cond;
}

static OFCondition echoSCP(
  T_ASC_Association * assoc,
  T_DIMSE_Message * msg,
  T_ASC_PresentationContextID presID)
{
  if (opt_verbose)
  {
    printf("Received ");
    DIMSE_printCEchoRQ(stdout, &msg->msg.CEchoRQ);
  }

  /* the echo succeeded !! */
  OFCondition cond = DIMSE_sendEchoResponse(assoc, presID, &msg->msg.CEchoRQ, STATUS_Success, NULL);
  if (cond.bad())
  {
    fprintf(stderr, "storescp: Echo SCP Failed:\n");
    DimseCondition::dump(cond);
  }
  return cond;
}

struct StoreCallbackData
{
  char* imageFileName;
  DcmFileFormat* dcmff;
  T_ASC_Association* assoc;
};


#if defined(VRN_DCMTK_VERSION_354)
static void
storeSCPCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName, DcmDataset **imageDataSet, /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **statusDetail)
{
    DIC_UI sopClass;
    DIC_UI sopInstance;

    if ((opt_abortDuringStore && progress->state != DIMSE_StoreBegin) ||
        (opt_abortAfterStore && progress->state == DIMSE_StoreEnd)) {
        if (opt_verbose) {
            printf("ABORT initiated (due to command line options)\n");
        }
        ASC_abortAssociation(((StoreCallbackData*) callbackData)->assoc);
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
        return;
    }

    if (opt_sleepDuring > 0)
    {
      OFStandard::sleep((unsigned int)opt_sleepDuring);
    }

    if (opt_verbose)
    {
      switch (progress->state)
      {
        case DIMSE_StoreBegin:
          printf("RECV:");
          break;
        case DIMSE_StoreEnd:
          printf("\n");
          break;
        default:
          putchar('.');
          break;
      }
      fflush(stdout);
    }

    if (progress->state == DIMSE_StoreEnd)
    {
       *statusDetail = NULL;    /* no status detail */

       /* could save the image somewhere else, put it in database, etc */
       /*
        * An appropriate status code is already set in the resp structure, it need not be success.
        * For example, if the caller has already detected an out of resources problem then the
        * status will reflect this.  The callback function is still called to allow cleanup.
        */
       // rsp->DimseStatus = STATUS_Success;

       if ((imageDataSet)&&(*imageDataSet)&&(!opt_bitPreserving)&&(!opt_ignore))
       {
         StoreCallbackData *cbdata = (StoreCallbackData*) callbackData;
         const char* fileName = cbdata->imageFileName;
         string s = filePath + "/" + fileName;
         if (!filePath.empty()) fileName = s.c_str();

         E_TransferSyntax xfer = opt_writeTransferSyntax;
         if (xfer == EXS_Unknown) xfer = (*imageDataSet)->getOriginalXfer();

         OFCondition cond = cbdata->dcmff->saveFile(fileName, xfer, opt_sequenceType, opt_groupLength,
           opt_paddingType, (Uint32)opt_filepad, (Uint32)opt_itempad, !opt_useMetaheader);
         if (cond.bad())
         {
           fprintf(stderr, "storescp: Cannot write image file: %s\n", fileName);
           rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
         }

        /* should really check the image to make sure it is consistent,
         * that its sopClass and sopInstance correspond with those in
         * the request.
         */
        if ((rsp->DimseStatus == STATUS_Success)&&(!opt_ignore))
        {
          /* which SOP class and SOP instance ? */
          if (! DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, opt_correctUIDPadding))
          {
             fprintf(stderr, "storescp: Bad image file: %s\n", imageFileName);
             rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
          }
          else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
          {
            rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
          }
          else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
          {
            rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
          }
          else
          {
              if (filelist != 0) {
                  filelist->push_back(string(fileName));
                  LINFO("DicomMoveSCU received " << fileName);
                  voreen::DcmtkMoveSCU::filesDone_++;
                  printf("Done: %i\n", voreen::DcmtkMoveSCU::filesDone_);
              }
          }
        }
      }
    }
    return;
}
#else
static void
storeSCPCallback(/* in */
                 void* /*callbackData*/,
                 T_DIMSE_StoreProgress* /*progress*/,    /* progress state */
                 T_DIMSE_C_StoreRQ /*req*/,             /* original store request */
                 char* /*imageFileName*/, DcmDataset /***imageDataSet*/, /* being received into */
                 /* out */
                 T_DIMSE_C_StoreRSP* /*rsp*/,            /* final store response */
                 DcmDataset* /*statusDetail*/)
{
    LWARNINGC("dicommovescu.cpp",
        "storeSCPCallback not supported for Dcmtk version " << OFFIS_DCMTK_VERSION_NUMBER);
}
#endif

#if defined(VRN_DCMTK_VERSION_354)
static OFCondition storeSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  T_ASC_PresentationContextID presID)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ *req;
    char imageFileName[2048];

    req = &msg->msg.CStoreRQ;

    if (opt_ignore)
    {
#ifdef _WIN32
        tmpnam(imageFileName);
#else
        strcpy(imageFileName, NULL_DEVICE_NAME);
#endif
    } else {
        sprintf(imageFileName, "%s.%s",
            dcmSOPClassUIDToModality(req->AffectedSOPClassUID),
            req->AffectedSOPInstanceUID);
    }

    if (opt_verbose)
    {
      printf("Received ");
      DIMSE_printCStoreRQ(stdout, req);
    }

    StoreCallbackData callbackData;
    callbackData.assoc = assoc;
    callbackData.imageFileName = imageFileName;
    DcmFileFormat dcmff;
    callbackData.dcmff = &dcmff;

    DcmDataset *dset = dcmff.getDataset();

    if (opt_bitPreserving)
    {
      cond = DIMSE_storeProvider(assoc, presID, req, imageFileName, opt_useMetaheader,
        NULL, storeSCPCallback, (void*)&callbackData, DIMSE_BLOCKING, 0);
    } else {
      cond = DIMSE_storeProvider(assoc, presID, req, (char *)NULL, opt_useMetaheader,
        &dset, storeSCPCallback, (void*)&callbackData, DIMSE_BLOCKING, 0);
    }

    if (cond.bad())
    {
      fprintf(stderr, "storescp: Store SCP Failed:\n");
      DimseCondition::dump(cond);
      /* remove file */
      if (!opt_ignore)
      {
        if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0) unlink(imageFileName);
      }
#ifdef _WIN32
    } else if (opt_ignore) {
        if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0) unlink(imageFileName); // delete the temporary file
#endif
    }

    if (opt_sleepAfter > 0)
    {
      OFStandard::sleep((unsigned int)opt_sleepAfter);
    }
    return cond;
}
#else
static OFCondition storeSCP(
    T_ASC_Association* /*assoc*/,
    T_DIMSE_Message* /*msg*/,
    T_ASC_PresentationContextID /*presID*/)
{
    LWARNINGC("dicommovescu.cpp",
          "storeSCP not supported for Dcmtk version " << OFFIS_DCMTK_VERSION_NUMBER);
    return EC_IllegalParameter;
}
#endif

static OFCondition
subOpSCP(T_ASC_Association **subAssoc)
{

    T_DIMSE_Message     msg;
    T_ASC_PresentationContextID presID;

    if (!ASC_dataWaiting(*subAssoc, 0)) /* just in case */
        return DIMSE_NODATAAVAILABLE;

    OFCondition cond = DIMSE_receiveCommand(*subAssoc, DIMSE_BLOCKING, 0, &presID,
            &msg, NULL);

    if (cond == EC_Normal) {
        switch (msg.CommandField) {
        case DIMSE_C_STORE_RQ:
            cond = storeSCP(*subAssoc, &msg, presID);
            break;
        case DIMSE_C_ECHO_RQ:
            cond = echoSCP(*subAssoc, &msg, presID);
            break;
        default:
            cond = DIMSE_BADCOMMANDTYPE;
            break;
        }
    }
    /* clean up on association termination */
    if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        cond = ASC_acknowledgeRelease(*subAssoc);
        ASC_dropSCPAssociation(*subAssoc);
        ASC_destroyAssociation(subAssoc);
        return cond;
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
    }
    else if (cond != EC_Normal)
    {
        errmsg("DIMSE Failure (aborting sub-association):\n");
        DimseCondition::dump(cond);
        /* some kind of error so abort the association */
        cond = ASC_abortAssociation(*subAssoc);
    }

    if (cond != EC_Normal)
    {
        ASC_dropAssociation(*subAssoc);
        ASC_destroyAssociation(subAssoc);
    }
    return cond;
}

static void
subOpCallback(void * /*subOpCallbackData*/ ,
        T_ASC_Network *aNet, T_ASC_Association **subAssoc)
{

    if (aNet == NULL) return;   /* help no net ! */

    if (*subAssoc == NULL) {
        /* negotiate association */
        acceptSubAssoc(aNet, subAssoc);
    } else {
        /* be a service class provider */
        subOpSCP(subAssoc);
    }
}

static void
moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request,
    int responseCount, T_DIMSE_C_MoveRSP *response)
{
    OFCondition cond = EC_Normal;
    MyCallbackInfo *myCallbackData;

    myCallbackData = (MyCallbackInfo*)callbackData;

    if (opt_verbose) {
        printf("Move Response %d: ", responseCount);
        DIMSE_printCMoveRSP(stdout, response);
    }
    /* should we send a cancel back ?? */
    if (opt_cancelAfterNResponses == responseCount) {
        if (opt_verbose) {
            printf("Sending Cancel RQ, MsgId: %d, PresId: %d\n",
                request->MessageID, myCallbackData->presId);
        }
        cond = DIMSE_sendCancelRequest(myCallbackData->assoc,
            myCallbackData->presId, request->MessageID);
        if (cond != EC_Normal) {
            errmsg("Cancel RQ Failed:");
            DimseCondition::dump(cond);
        }
    }
}


static void
substituteOverrideKeys(DcmDataset *dset)
{
    if (overrideKeys == NULL) {
        return; /* nothing to do */
    }

    /* copy the override keys */
    DcmDataset keys(*overrideKeys);

    /* put the override keys into dset replacing existing tags */
    unsigned long elemCount = keys.card();
    for (unsigned long i=0; i<elemCount; i++) {
        DcmElement *elem = keys.remove((unsigned long)0);

        dset->insert(elem, OFTrue);
    }
}

////////////////////////////////////////////

// With SSL enabled selectReadable() introduces massive delays between transfer of files,
// therefore we switch to DIMSE_NONBLOCKING, but only for selectReadable(), not
// DIMSE_receiveCommand(). So we have to repeat the code from dcmnet/libsrc/dimmove.cc here
// with only changing the parameters on the call to selectReadable().

namespace {

static int
selectReadable(T_ASC_Association *assoc,
    T_ASC_Network *net, T_ASC_Association *subAssoc,
    T_DIMSE_BlockingMode blockMode, int timeout)
{
    T_ASC_Association *assocList[2];
    int assocCount = 0;

    if (net != NULL && subAssoc == NULL) {
        if (ASC_associationWaiting(net, 0)) {
            /* association request waiting on network */
            return 2;
        }
    }
    assocList[0] = assoc;
    assocCount = 1;
    assocList[1] = subAssoc;
    if (subAssoc != NULL) assocCount++;
    if (subAssoc == NULL) {
        timeout = 1;    /* poll wait until an assoc req or move rsp */
    } else {
        if (blockMode == DIMSE_BLOCKING) {
            timeout = 10000;    /* a long time */
        }
    }

    if (!ASC_selectReadableAssociation(assocList, assocCount, timeout)) {
        /* none readable */
        return 0;
    }
    if (assocList[0] != NULL) {
        /* main association readable */
        return 1;
    }
    if (assocList[1] != NULL) {
        /* sub association readable */
        return 2;
    }
    /* should not be reached */
    return 0;
}

OFCondition
myDIMSE_moveUser(
        /* in */
        T_ASC_Association *assoc,
        T_ASC_PresentationContextID presID,
        T_DIMSE_C_MoveRQ *request,
        DcmDataset *requestIdentifiers,
        DIMSE_MoveUserCallback callback, void *callbackData,
        /* blocking info for response */
        T_DIMSE_BlockingMode blockMode, int timeout,
        /* sub-operation provider callback */
        T_ASC_Network *net,
        DIMSE_SubOpProviderCallback subOpCallback, void *subOpCallbackData,
        /* out */
        T_DIMSE_C_MoveRSP *response, DcmDataset **statusDetail,
        DcmDataset **rspIds,
        OFBool ignorePendingDatasets)
{
    T_DIMSE_Message req, rsp;
    DIC_US msgId;
    int responseCount = 0;
    T_ASC_Association *subAssoc = NULL;
    DIC_US status = STATUS_Pending;

    if (requestIdentifiers == NULL) return DIMSE_NULLKEY;

    bzero((char*)&req, sizeof(req));
    bzero((char*)&rsp, sizeof(rsp));

    req.CommandField = DIMSE_C_MOVE_RQ;
    request->DataSetType = DIMSE_DATASET_PRESENT;
    req.msg.CMoveRQ = *request;

    msgId = request->MessageID;

    OFCondition cond = DIMSE_sendMessageUsingMemoryData(assoc, presID, &req,
                                          NULL, requestIdentifiers,
                                          NULL, NULL);
    if (cond != EC_Normal) {
        return cond;
    }

    /* receive responses */

    while (cond == EC_Normal && status == STATUS_Pending) {

        /* if user wants, multiplex between net/subAssoc
         * and move responses over main assoc.
         */
        switch (selectReadable(assoc, net, subAssoc, DIMSE_NONBLOCKING /*blockMode*/, timeout)) {
        case 0:
            /* none are readble, timeout */
            if (blockMode == DIMSE_BLOCKING) {
                continue;       /* continue with while loop */
            } else {
                return DIMSE_NODATAAVAILABLE;
            }
            /* break; */ // never reached after continue or return.
        case 1:
            /* main association readable */
            break;
        case 2:
            /* net/subAssoc readable */
            if (subOpCallback) {
                subOpCallback(subOpCallbackData, net, &subAssoc);
            }
            continue;   /* continue with main loop */
            /* break; */ // never reached after continue statement
        }

        bzero((char*)&rsp, sizeof(rsp));

        cond = DIMSE_receiveCommand(assoc, blockMode, timeout, &presID,
                &rsp, statusDetail);
        if (cond != EC_Normal) {
            return cond;
        }
        if (rsp.CommandField != DIMSE_C_MOVE_RSP)
        {
          char buf1[256];
          sprintf(buf1, "DIMSE: Unexpected Response Command Field: 0x%x", (unsigned)rsp.CommandField);
          return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf1);
        }

        *response = rsp.msg.CMoveRSP;

        if (response->MessageIDBeingRespondedTo != msgId)
        {
          char buf2[256];
          sprintf(buf2, "DIMSE: Unexpected Response MsgId: %d (expected: %d)", response->MessageIDBeingRespondedTo, msgId);
          return makeDcmnetCondition(DIMSEC_UNEXPECTEDRESPONSE, OF_error, buf2);
        }

        status = response->DimseStatus;
        responseCount++;

        switch (status) {
        case STATUS_Pending:
            if (*statusDetail != NULL) {
#if defined(VRN_DCMTK_VERSION_354)
                DIMSE_warning(assoc,
                    "moveUser: Pending with statusDetail, ignoring detail");
#endif
                delete *statusDetail;
                *statusDetail = NULL;
            }
            if (response->DataSetType != DIMSE_DATASET_NULL)
            {
#if defined(VRN_DCMTK_VERSION_354)
                DIMSE_warning(assoc, "moveUser: Status Pending, but DataSetType!=NULL");
#endif
                if (! ignorePendingDatasets)
                {
                    // Some systems send an (illegal) dataset following C-MOVE-RSP messages
                    // with pending status, which is a protocol violation, but we need to
                    // handle this nevertheless. The MV300 has been reported to exhibit
                    // this behavior.
#if defined(VRN_DCMTK_VERSION_354)
                    DIMSE_warning(assoc, "  Reading but ignoring response identifier set");
#endif
                    DcmDataset *tempset = NULL;
                    cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout, &presID, &tempset, NULL, NULL);
                    delete tempset;
                    if (cond != EC_Normal) {
                        return cond;
                    }
                }
                else
                {
                    // The alternative is to assume that the command set is wrong
                    // and not to read a dataset from the network association.
#if defined(VRN_DCMTK_VERSION_354)
                    DIMSE_warning(assoc, "  Assuming NO response identifiers are present");
#endif
                }
            }

            /* execute callback */
            if (callback) {
                callback(callbackData, request, responseCount, response);
            }
            break;
        default:
            if (response->DataSetType != DIMSE_DATASET_NULL) {
                cond = DIMSE_receiveDataSetInMemory(assoc, blockMode, timeout,
                    &presID, rspIds, NULL, NULL);
                if (cond != EC_Normal) {
                    return cond;
                }
            }
            break;
        }
    }

    /* do remaining sub-association work, we may receive a non-pending
     * status before the sub-association has cleaned up.
     */
    while (subAssoc != NULL) {
        if (subOpCallback) {
            subOpCallback(subOpCallbackData, net, &subAssoc);
        }
    }

    return cond;
}

} // namespace

/////////////////////////////////////////

static  OFCondition
moveSCU(T_ASC_Association * assoc, const char *fname)
{
    T_ASC_PresentationContextID presId;
    T_DIMSE_C_MoveRQ    req;
    T_DIMSE_C_MoveRSP   rsp;
    DIC_US              msgId = assoc->nextMsgID++;
    DcmDataset          *rspIds = NULL;
    const char          *sopClass;
    DcmDataset          *statusDetail = NULL;
    MyCallbackInfo      callbackData;

    if (opt_verbose) {
        printf("================================\n");
        if (fname) printf("Sending query file: %s\n", fname); else printf("Sending query\n");
    }

    DcmFileFormat dcmff;

    if (fname != NULL) {
        if (dcmff.loadFile(fname).bad()) {
            errmsg("Bad DICOM file: %s: %s", fname, dcmff.error().text());
            return DIMSE_BADDATA;
        }
    }

    /* replace specific keys by those in overrideKeys */
    substituteOverrideKeys(dcmff.getDataset());

    sopClass = querySyntax[opt_queryModel].moveSyntax;

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);
    if (presId == 0) return DIMSE_NOVALIDPRESENTATIONCONTEXTID;

    if (opt_verbose) {
        printf("Move SCU RQ: MsgID %d\n", msgId);
        printf("Request:\n");
        dcmff.getDataset()->print(COUT);
    }

    callbackData.assoc = assoc;
    callbackData.presId = presId;

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    req.Priority = DIMSE_PRIORITY_MEDIUM;
    req.DataSetType = DIMSE_DATASET_PRESENT;
    if (opt_moveDestination == NULL) {
        /* set the destination to be me */
        ASC_getAPTitles(assoc->params, req.MoveDestination,
            NULL, NULL);
    } else {
        strcpy(req.MoveDestination, opt_moveDestination);
    }

    OFCondition cond = myDIMSE_moveUser(assoc, presId, &req, dcmff.getDataset(),
        moveCallback, &callbackData, DIMSE_BLOCKING, 0,
        net, subOpCallback, NULL,
        &rsp, &statusDetail, &rspIds, OFTrue);

    if (cond == EC_Normal) {
        if (opt_verbose) {
            DIMSE_printCMoveRSP(stdout, &rsp);
            if (rspIds != NULL) {
                printf("Response Identifiers:\n");
                rspIds->print(COUT);
            }
        }
    } else {
        errmsg("Move Failed:");
        DimseCondition::dump(cond);
    }
    if (statusDetail != NULL) {
        printf("  Status Detail:\n");
        statusDetail->print(COUT);
        delete statusDetail;
    }

    if (rspIds != NULL) delete rspIds;

    return cond;
}


static OFCondition
cmove(T_ASC_Association * assoc, const char *fname)
{
    OFCondition cond = EC_Normal;
    int n = (int)opt_repeatCount;

    while (cond.good() && n--)
    {
        cond = moveSCU(assoc, fname);
    }
    return cond;
}

} // namespace


bool voreen::DcmtkMoveSCU::init(const std::string& ourtitle, int retrievePort,
                                const std::string& peer, int port,
                                const DcmtkSecurityOptions& security,
                                const std::string& configFile)
{
    if (ourtitle.empty() || retrievePort <= 0 || peer.empty() || port <= 0)
        return false;

    ourtitle_ = ourtitle;
    retrievePort_ = retrievePort;
    peer_ = peer;
    port_ = port;
    security_ = security;
    opt_configFile = configFile;
    filesDone_ = 0;

    return true;
}

bool voreen::DcmtkMoveSCU::init(const std::string& url, const DcmtkSecurityOptions& security,
                                const std::string& configFile)
{
    // First extract the URL parts like in this regex:
    // ^dicom://([a-zA-Z0-9\-.]+):([0-9]+)@([a-zA-Z0-9\-\.]+):([0-9]+)$

    const string number = "0123456789";
    const string word = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-." + number;
    string ourtitle, peer;
    int retrievePort, port;
    string s(url);

    // ^dicom://
    if (s.find("dicom://") != 0) return false;
    s = s.substr(8);

    // ([a-zA-Z0-9\-.]+):
    size_t pos = s.find_first_not_of(word);
    if (pos <= 0) return false;
    ourtitle = s.substr(0, pos);
    if (s.substr(pos, 1) != ":") return false;
    s = s.substr(pos + 1);

    // ([0-9]+)@
    pos = s.find_first_not_of(number);
    if (pos <= 0) return false;
    retrievePort = atoi(s.substr(0, pos).c_str());
    if (s.substr(pos, 1) != "@") return false;
    s = s.substr(pos + 1);

    // ([a-zA-Z0-9\-.]+):
    pos = s.find_first_not_of(word);
    if (pos <= 0) return false;
    peer = s.substr(0, pos);
    if (s.substr(pos, 1) != ":") return false;
    s = s.substr(pos + 1);

    // ([0-9]+)$
    pos = s.find_first_not_of(number);
    if (pos != string::npos) return false;
    port = atoi(s.c_str());

    return init(ourtitle, retrievePort, peer, port, security, configFile);
}

int voreen::DcmtkMoveSCU::move(std::vector<std::string>& keys, QueryModel queryModel,
                       const std::string& peerTitle, std::vector<std::string>* files)
{
    opt_secureConnection = (security_.secureConnection_ ? OFTrue : OFFalse);
    if (asccfg) delete asccfg;
    asccfg = new DcmAssociationConfiguration();

#ifdef WITH_OPENSSL
    int         opt_keyFileFormat = SSL_FILETYPE_PEM;
    OFBool      opt_doAuthenticate = (security_.authenticate_ ? OFTrue : OFFalse);
    const char *opt_privateKeyFile = (security_.privateKeyFile_.empty() ? NULL : security_.privateKeyFile_.c_str());
    const char *opt_certificateFile = (security_.publicKeyFile_.empty() ? NULL : security_.publicKeyFile_.c_str());
    const char *opt_passwd = NULL;
    OFString    opt_ciphersuites(SSL3_TXT_RSA_DES_192_CBC3_SHA);
    const char *opt_readSeedFile = NULL;
    //const char *opt_writeSeedFile = NULL;
    DcmCertificateVerification opt_certVerification = DCV_requireCertificate;
    //const char *opt_dhparam = NULL;
#endif

    if (port_ <= 0) {
        errmsg("DicomMoveSCU not initialized.");
        return 1;
    }

    //
    // Setup parameters
    //
    vector<string>::iterator iter = keys.begin();
    while (iter != keys.end()) {
        addOverrideKey((*iter).c_str());
        iter++;
    }
    opt_queryModel = queryModel;
    const char *opt_peerTitle = peerTitle.c_str();
    filelist = files;

    const char *opt_peer = peer_.c_str();
    OFCmdUnsignedInt opt_port = port_;
    const char *opt_ourTitle = ourtitle_.c_str();
    opt_retrievePort = retrievePort_;

//    opt_verbose = OFTrue;
#if defined(VRN_DCMTK_VERSION_354)
    SetDebugLevel((0)); /* stop dcmdata debugging messages */
#endif


//     if (cmd.findOption("--prefer-uncompr"))  opt_in_networkTransferSyntax = EXS_Unknown;
//     if (cmd.findOption("--prefer-little"))   opt_in_networkTransferSyntax = EXS_LittleEndianExplicit;
//     if (cmd.findOption("--prefer-big"))      opt_in_networkTransferSyntax = EXS_BigEndianExplicit;
//     if (cmd.findOption("--prefer-lossless")) opt_in_networkTransferSyntax = EXS_JPEGProcess14SV1TransferSyntax;
//     if (cmd.findOption("--prefer-jpeg8"))    opt_in_networkTransferSyntax = EXS_JPEGProcess1TransferSyntax;
//     if (cmd.findOption("--prefer-jpeg12"))   opt_in_networkTransferSyntax = EXS_JPEGProcess2_4TransferSyntax;
//     if (cmd.findOption("--prefer-rle"))      opt_in_networkTransferSyntax = EXS_RLELossless;
//     if (cmd.findOption("--implicit"))        opt_in_networkTransferSyntax = EXS_LittleEndianImplicit;
//     if (cmd.findOption("--propose-uncompr"))  opt_out_networkTransferSyntax = EXS_Unknown;
//     if (cmd.findOption("--propose-little"))   opt_out_networkTransferSyntax = EXS_LittleEndianExplicit;
//     if (cmd.findOption("--propose-big"))      opt_out_networkTransferSyntax = EXS_BigEndianExplicit;
//     if (cmd.findOption("--propose-implicit")) opt_out_networkTransferSyntax = EXS_LittleEndianImplicit;

    opt_in_networkTransferSyntax = EXS_JPEGProcess14SV1TransferSyntax;

    if (!opt_configFile.empty()) {
        // read configuration file
        OFCondition cond = DcmAssociationConfigurationFile::initialize(*asccfg, opt_configFile.c_str());
        if (cond.bad())
        {
            CERR << "error reading config file: "
                << cond.text() << std::endl;
            return 1;
        }

        /* perform name mangling for config file key */
        OFString sprofile;
        const char *c = opt_profileName.c_str();
        while (*c)
        {
            if (! isspace(*c)) sprofile += OFstatic_cast(char, toupper(*c));
            ++c;
        }

        if (!asccfg->isKnownProfile(sprofile.c_str()))
        {
            CERR << "unknown configuration profile name: "
                << sprofile << std::endl;
            return 1;
        }

        if (!asccfg->isValidSCPProfile(sprofile.c_str()))
        {
            CERR << "profile '"
                 << sprofile
                 << "' is not valid for SCP use, duplicate abstract syntaxes found."
                 << std::endl;
            return 1;
        }
    }


    T_ASC_Parameters *params = NULL;
    DIC_NODENAME localHost;
    DIC_NODENAME peerHost;
    T_ASC_Association *assoc = NULL;
    OFList<OFString> fileNameList;


#ifdef HAVE_GUSI_H
    /* needed for Macintosh */
    GUSISetup(GUSIwithSIOUXSockets);
    GUSISetup(GUSIwithInternetSockets);
#endif

#ifdef HAVE_WINSOCK_H
    WSAData winSockData;
    /* we need at least version 1.1 */
    WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
    WSAStartup(winSockVersionNeeded, &winSockData);
#endif

//     if (debug) {
//         opt_debug = OFTrue;
//         DUL_Debug(OFTrue);
//         DIMSE_debug(OFTrue);
//         SetDebugLevel(3);
//     }



    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        fprintf(stderr, "Warning: no data dictionary loaded, check environment variable: %s\n",
                DCM_DICT_ENVIRONMENT_VARIABLE);
    }

#ifdef HAVE_GETEUID
    /* if retrieve port is privileged we must be as well */
    if (opt_retrievePort < 1024) {
        if (geteuid() != 0) {
            errmsg("cannot listen on port %d, insufficient privileges", opt_retrievePort);
            return 1;
        }
    }
#endif

    /* network for move request and responses */
    OFCondition cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_retrievePort,
                                             1000, &net);
    if (cond.bad())
    {
        errmsg("cannot create network:");
        DimseCondition::dump(cond);
        return 1;
    }

#ifdef WITH_OPENSSL

    DcmTLSTransportLayer *tLayer = NULL;
    if (opt_secureConnection)
    {
      tLayer = new DcmTLSTransportLayer(DICOM_APPLICATION_ACCEPTOR | DICOM_APPLICATION_REQUESTOR, opt_readSeedFile);
      if (tLayer == NULL)
      {
          std::cerr << "unable to create TLS transport layer" << std::endl;
      }

      for (size_t i=0; i < security_.certificateFiles_.size(); i++)
          tLayer->addTrustedCertificateFile(security_.certificateFiles_[i].c_str(), opt_keyFileFormat);

      if (opt_doAuthenticate)
      {
        if (opt_passwd) tLayer->setPrivateKeyPasswd(opt_passwd);

        if (TCS_ok != tLayer->setPrivateKeyFile(opt_privateKeyFile, opt_keyFileFormat))
        {
          CERR << "unable to load private TLS key from '" << opt_privateKeyFile << "'" << endl;
          return 1;
        }
        if (TCS_ok != tLayer->setCertificateFile(opt_certificateFile, opt_keyFileFormat))
        {
          CERR << "unable to load certificate from '" << opt_certificateFile << "'" << endl;
          return 1;
        }
        if (! tLayer->checkPrivateKeyMatchesCertificate())
        {
          CERR << "private key '" << opt_privateKeyFile << "' and certificate '"
               << opt_certificateFile << "' do not match" << endl;
          return 1;
        }
      }

      if (TCS_ok != tLayer->setCipherSuites(opt_ciphersuites.c_str()))
      {
        CERR << "unable to set selected cipher suites" << endl;
        return 1;
      }

      tLayer->setCertificateVerification(opt_certVerification);


      cond = ASC_setTransportLayer(net, tLayer, 0);
      if (cond.bad())
      {
          DimseCondition::dump(cond);
          return 1;
      }
    }

#endif // WITH_OPENSSL

#ifdef HAVE_GETUID
    /* return to normal uid so that we can't do too much damage in case
     * things go very wrong.   Only does someting if the program is setuid
     * root, and run by another user.  Running as root user may be
     * potentially disasterous if this program screws up badly.
     */
    setuid(getuid());
#endif

    /* set up main association */
    cond = ASC_createAssociationParameters(&params, opt_maxPDU);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        return 1;
    }
    ASC_setAPTitles(params, opt_ourTitle, opt_peerTitle, NULL);

    gethostname(localHost, sizeof(localHost) - 1);
    sprintf(peerHost, "%s:%d", opt_peer, (int)opt_port);
    ASC_setPresentationAddresses(params, localHost, peerHost);

    /* Set the transport layer type (type of network connection) in the params */
    /* structure. The default is an insecure connection; where OpenSSL is  */
    /* available the user is able to request an encrypted,secure connection. */
    cond = ASC_setTransportLayerType(params, opt_secureConnection);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        return 1;
    }

    /*
     * We also add a presentation context for the corresponding
     * find sop class.
     */
    cond = addPresentationContext(params, 1,
                                  querySyntax[opt_queryModel].findSyntax);

    cond = addPresentationContext(params, 3,
                                  querySyntax[opt_queryModel].moveSyntax);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        return 1;
    }
    if (opt_debug) {
        printf("Request Parameters:\n");
        ASC_dumpParameters(params, COUT);
    }

    /* create association */
    if (opt_verbose)
        printf("Requesting Association\n");
    cond = ASC_requestAssociation(net, params, &assoc);
    if (cond.bad()) {
        if (cond == DUL_ASSOCIATIONREJECTED) {
            T_ASC_RejectParameters rej;

            ASC_getRejectParameters(params, &rej);
            errmsg("Association Rejected:");
            ASC_printRejectParameters(stderr, &rej);
            return 1;
        } else {
            errmsg("Association Request Failed:");
            DimseCondition::dump(cond);
            return 1;
        }
    }
    /* what has been accepted/refused ? */
    if (opt_debug) {
        printf("Association Parameters Negotiated:\n");
        ASC_dumpParameters(params, COUT);
    }

    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        errmsg("No Acceptable Presentation Contexts");
        return 1;
    }

    if (opt_verbose) {
        printf("Association Accepted (Max Send PDV: %lu)\n",
               assoc->sendPDVLength);
    }

    /* do the real work */
    cond = EC_Normal;
    if (fileNameList.empty())
    {
        /* no files provided on command line */
        cond = cmove(assoc, NULL);
    } else {
        OFListIterator(OFString) iter = fileNameList.begin();
        OFListIterator(OFString) enditer = fileNameList.end();
        while ((iter != enditer) && (cond == EC_Normal)) // compare with EC_Normal since DUL_PEERREQUESTEDRELEASE is also good()
        {

            cond = cmove(assoc, (*iter).c_str());
            ++iter;
        }
    }

    /* tear down association */
    if (cond == EC_Normal)
    {
        if (opt_abortAssociation) {
            if (opt_verbose)
                printf("Aborting Association\n");
            cond = ASC_abortAssociation(assoc);
            if (cond.bad()) {
                errmsg("Association Abort Failed:");
                DimseCondition::dump(cond);
                return 1;
            }
        } else {
            /* release association */
            if (opt_verbose)
                printf("Releasing Association\n");
            cond = ASC_releaseAssociation(assoc);
            if (cond.bad())
            {
                errmsg("Association Release Failed:");
                DimseCondition::dump(cond);
                return 1;
            }
        }
    }
    else if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        errmsg("Protocol Error: peer requested release (Aborting)");
        if (opt_verbose)
            printf("Aborting Association\n");
        cond = ASC_abortAssociation(assoc);
        if (cond.bad()) {
            errmsg("Association Abort Failed:");
            DimseCondition::dump(cond);
            return 1;
        }
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
        if (opt_verbose) printf("Peer Aborted Association\n");
    }
    else
    {
        errmsg("SCU Failed:");
        DimseCondition::dump(cond);
        if (opt_verbose)
            printf("Aborting Association\n");
        cond = ASC_abortAssociation(assoc);
        if (cond.bad()) {
            errmsg("Association Abort Failed:");
            DimseCondition::dump(cond);
            return 1;
        }
    }

    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        return 1;
    }
    cond = ASC_dropNetwork(&net);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        return 1;
    }

#ifdef HAVE_WINSOCK_H
    WSACleanup();
#endif

#ifdef WITH_OPENSSL
    delete tLayer;
#endif

    delete asccfg;
    asccfg = 0;

    return 0;
}

int voreen::DcmtkMoveSCU::moveSeries(const std::string& seriesInstanceUID, const std::string& peerTitle,
                             std::vector<std::string>* files, std::string targetPath) {
    vector<string> keys;
    keys.push_back("0008,0052=SERIES");
    keys.push_back("0010,0020=");
    keys.push_back("0020,000d=");
    keys.push_back("0020,000e=" + seriesInstanceUID);

    filePath = targetPath;

    return move(keys, QMPatientRoot, peerTitle, files);
}

void voreen::DcmtkMoveSCU::cancelMove() {
    opt_abortDuringStore = OFTrue;
    opt_abortAfterStore = OFTrue;
}

int voreen::DcmtkMoveSCU::getNumDownloaded() {
    return filesDone_;
}
