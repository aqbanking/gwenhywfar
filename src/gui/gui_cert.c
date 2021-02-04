/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2021 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

/* included from gui.c */




int GWEN_Gui_CheckCert(const GWEN_SSLCERTDESCR *cd, GWEN_SYNCIO *sio, uint32_t guiid)
{
  GWEN_GUI *gui;

  gui=GWEN_Gui_GetGui();
  if (gui && gui->checkCertFn)
    return gui->checkCertFn(gui, cd, sio, guiid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWENHYWFAR_CB GWEN_Gui_CheckCertBuiltIn(GWEN_UNUSED GWEN_GUI *gui,
                                            const GWEN_SSLCERTDESCR *cd,
                                            GWEN_UNUSED GWEN_SYNCIO *sio, uint32_t guiid)
{
  int rv;
  int isError;
  const char *hashMd5;
  const char *hashSha1;
  const char *hashSha512;
  const char *status;
  //const char *ipAddr;
  const char *statusOn;
  const char *statusOff;
  char varName[128];
  char dbuffer1[32];
  char dbuffer2[32];
  char buffer[8192];
  const GWEN_TIME *ti;
  const char *unknown;
  const char *commonName;
  const char *organizationName;
  const char *organizationalUnitName;
  const char *countryName;
  const char *localityName;
  const char *stateOrProvinceName;

  const char *msg=I18S(
                       "The following certificate has been received:\n"
                       "Name         : %s\n"
                       "Organisation : %s\n"
                       "Department   : %s\n"
                       "Country      : %s\n"
                       "City         : %s\n"
                       "State        : %s\n"
                       "Valid after  : %s\n"
                       "Valid until  : %s\n"
                       "Hash (MD5)   : %s\n"
                       "Hash (SHA1)  : %s\n"
                       "Hash (SHA512): %s\n"
                       "Status       : %s\n"
                       "Do you wish to accept this certificate?"

                       "<html>"
                       " <p>"
                       "  The following certificate has been received:"
                       " </p>"
                       " <table>"
                       "  <tr><td>Name</td><td>%s</td></tr>"
                       "  <tr><td>Organisation</td><td>%s</td></tr>"
                       "  <tr><td>Department</td><td>%s</td></tr>"
                       "  <tr><td>Country</td><td>%s</td></tr>"
                       "  <tr><td>City</td><td>%s</td></tr>"
                       "  <tr><td>State</td><td>%s</td></tr>"
                       "  <tr><td>Valid after</td><td>%s</td></tr>"
                       "  <tr><td>Valid until</td><td>%s</td></tr>"
                       "  <tr><td>Hash (MD5)</td><td>%s</td></tr>"
                       "  <tr><td>Hash (SHA1)</td><td>%s</td></tr>"
                       "  <tr><td>Hash (SHA512)</td><td>%s</td></tr>"
                       "  <tr><td>Status</td><td>%s%s%s</td></tr>"
                       " </table>"
                       " <p>"
                       "  Do you wish to accept this certificate?"
                       " </p>"
                       "</html>"
                      );

  memset(dbuffer1, 0, sizeof(dbuffer1));
  memset(dbuffer2, 0, sizeof(dbuffer2));
  memset(varName, 0, sizeof(varName));

  isError=GWEN_SslCertDescr_GetIsError(cd);

  hashMd5=GWEN_SslCertDescr_GetFingerPrint(cd);
  hashSha1=GWEN_SslCertDescr_GetFingerPrintSha1(cd);
  hashSha512=GWEN_SslCertDescr_GetFingerPrintSha512(cd);
  status=GWEN_SslCertDescr_GetStatusText(cd);
  //ipAddr=GWEN_SslCertDescr_GetIpAddress(cd);

  ti=GWEN_SslCertDescr_GetNotBefore(cd);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    /* TRANSLATORS: This string is used as a template string to
       convert a given time into your local translated timeformat. The
       following characters are accepted in the template string: Y -
       digit of the year, M - digit of the month, D - digit of the day
       of month, h - digit of the hour, m - digit of the minute, s-
       digit of the second. All other characters are left unchanged. */
    if (GWEN_Time_toString(ti, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert beforeDate to string");
      abort();
    }
    strncpy(dbuffer1, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer1)-1);
    GWEN_Buffer_free(tbuf);
  }

  ti=GWEN_SslCertDescr_GetNotAfter(cd);
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(ti, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert untilDate to string");
      abort();
    }
    strncpy(dbuffer2, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer2)-1);
    GWEN_Buffer_free(tbuf);
  }

  if (isError) {
    statusOn="<font color=red>";
    statusOff="</font>";
  }
  else {
    statusOn="<font color=green>";
    statusOff="</font>";
  }

  unknown=I18N("unknown");
  commonName=GWEN_SslCertDescr_GetCommonName(cd);
  if (!commonName)
    commonName=unknown;
  organizationName=GWEN_SslCertDescr_GetOrganizationName(cd);
  if (!organizationName)
    organizationName=unknown;
  organizationalUnitName=GWEN_SslCertDescr_GetOrganizationalUnitName(cd);
  if (!organizationalUnitName)
    organizationalUnitName=unknown;
  countryName=GWEN_SslCertDescr_GetCountryName(cd);
  if (!countryName)
    countryName=unknown;
  localityName=GWEN_SslCertDescr_GetLocalityName(cd);
  if (!localityName)
    localityName=unknown;
  stateOrProvinceName=GWEN_SslCertDescr_GetStateOrProvinceName(cd);
  if (!stateOrProvinceName)
    stateOrProvinceName=unknown;
  if (!status)
    status=unknown;

  snprintf(buffer, sizeof(buffer)-1,
           I18N(msg),
           commonName,
           organizationName,
           organizationalUnitName,
           countryName,
           localityName,
           stateOrProvinceName,
           dbuffer1, dbuffer2,
           hashMd5,
           hashSha1,
           hashSha512,
           status,
           /* the same again for HTML */
           commonName,
           organizationName,
           organizationalUnitName,
           countryName,
           localityName,
           stateOrProvinceName,
           dbuffer1, dbuffer2,
           hashMd5,
           hashSha1,
           hashSha512,
           statusOn,
           status,
           statusOff
          );

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
                         GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
                         GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                         I18N("Certificate Received"),
                         buffer,
                         I18N("Yes"), I18N("No"), 0, guiid);
  if (rv==1) {
    return 0;
  }
  else {
    DBG_NOTICE(GWEN_LOGDOMAIN, "User rejected certificate");

    return GWEN_ERROR_SSL_SECURITY;
  }
}



