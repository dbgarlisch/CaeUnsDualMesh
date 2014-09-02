/****************************************************************************
 *
 * Pointwise Plugin utility functions
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _RTPWPPLUGININFO_H_
#define _RTPWPPLUGININFO_H_

/*! \cond */

    /* initialize the PWP_PLUGININFO data returned by
       PwpGetPluginInfo(PWP_PLUGININFO *pInfo)
    */
    VERSION_PWP_INIT,         // conforms to this PWP-API version
    VERSION_LIB_INIT,         // software library release version
    "Pointwise, Inc.",        // company/author description
    "support.pointwise.com",  // support description (phone, web-link).
    "Copyright(c) 2008-2013", // copyright description
    0,                        // number of APIs (auto-set at runtime)
    0,                        // default msg callback (auto-set at runtime)
    0,                        // spy msg callback (auto-set at runtime)

/*! \endcond */

#endif /* _RTPWPPLUGININFO_H_ */
