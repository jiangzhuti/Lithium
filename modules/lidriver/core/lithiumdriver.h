/** LITHIUM Driver Functions
 *  Copyright (C) 2003 - 2015 Jasem Mutlaq
 *  Copyright (C) 2003 - 2006 Elwood C. Downey
 *
 *  This library is free software;
 *  you can redistribute it and / or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation;
 *  either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library;
 *  if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA
 */

#pragma once

#include "lithiumapi.h"
#include "lilxml.h"
#include "lithiumcom.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    extern int verbose; /* chatty */
    extern char *me;    /* a.out name */
    // extern LilXML *clixml; /* XML parser context */

    extern int dispatch(XMLEle *root, char msg[]);
    // extern void clientMsgCB(int fd, void *arg);

    /**
     * @defgroup configFunctions Configuration Functions: Functions drivers call to save and load configuraion options.
     *
     * <p>Drivers can save properties states and values in an XML configuration file. The following functions take an optional filename
     * parameter which specifies the full path of the configuration file. If the filename is set to NULL, the configuration file
     * is locally stored in ~/.indi. By default, two configuration files may exist for each driver:</p>
     * <ul>
     * <li>Last Saved Configuration: ~/.indi/driver_name_config.xml</li>
     * <li>Default Configuration: ~/.indi/driver_name_config.xml.default</li>
     * </ul>
     *
     * <p>libindi stores the configuration parameters enclosed in newXXX commands. Therefore, if a configuration file is loaded,
     * the driver property gets updated as if a client is setting these values. This is important to note since some configuration
     * options may only available when the device is powered up or is in a particular state.</p>
     *
     * <p>If no filename is supplied, each function will try to create the configuration files in the following order:</p>
     * <ol>
     * <li>LITHIUMCONFIG environment variable: The functions checks if the envrionment variable is defined, and if so, it shall
     * be used as the configuration filename</li>
     * <li>Generate filename: If the <i>device_name</i> is supplied, the function will attempt to set the configuration filename to ~/.indi/device_name_config.xml</li>
     * </ol>
     * @author Jasem Mutlaq
     * @note Drivers subclassing LITHIUM::DefaultDevice do not need to call the configuration functions directly as it is handled internally by the class.
     * @version libindi 1.1+
     */

    /* @{ */

    /** @brief Open a configuration file for writing and return a configuration file FILE pointer.
     *  @param filename full path of the configuration file. If set, the function will attempt to open it for writing.
     *         If set to NULL, it will attempt to generate the filename as described in the <b>Detailed Description</b> introduction and then open it for writing.
     *  @param dev device name. This is used if the filename parameter is NULL, and LITHIUMCONFIG environment variable is not set as described in the <b>Detailed Description</b> introduction.
     *  @param mode mode to open the file with (e.g. "w" or "r")
     *  @param errmsg In case of errors, store the error message in this buffer. The size of the buffer must be at least MAXRBUF.
     *  @return pointer to FILE if configuration file is opened successful, otherwise NULL and errmsg is set.
     */
    extern FILE *IUGetConfigFP(const char *filename, const char *dev, const char *mode, char errmsg[]);

    /**
     *  @param filename full path of the configuration file. If set, it will be deleted from disk.
     *         If set to NULL, it will attempt to generate the filename as described in the <b>Detailed Description</b> introduction and then delete it.
     *  @param dev device name. This is used if the filename parameter is NULL, and LITHIUMCONFIG environment variable is not set as described in the <b>Detailed Description</b> introduction.
     *  @param errmsg In case of errors, store the error message in this buffer. The size of the buffer must be at least MAXRBUF.
     *  @return 0 on success, -1 on failure.
     */
    extern int IUPurgeConfig(const char *filename, const char *dev, char errmsg[]);

    /** @brief Loads and processes a configuration file.
     *  Once a configuration file is successful loaded, the function will iterate over the enclosed newXXX commands, and dispatches
     *  each command to the driver. Subsequently, the driver receives the updated property value in the driver's ISNewXXX functions.
     *  The driver may call this function at any time. However, it is usually called either on driver startup or on device power up.
     *  By default, all the properties are read from the configuration file. To load a specific property, pass the property name, otherwise
     *  pass NULL to retrieve all properties.
     *  @param filename full path of the configuration file. If set, the function will attempt to load the file.
     *         If set to NULL, it will attempt to generate the filename as described in the <b>Detailed Description</b> introduction and then load it.
     *  @param dev device name. This is used if the filename parameter is NULL, and LITHIUMCONFIG environment variable is not set as described in the <b>Detailed Description</b> introduction.
     *  @param property Property name to load configuration for. If NULL, all properties within the configuration file will be processed.
     *  @param silent If silent is 1, it will suppress any output messages to the driver.
     *  @param errmsg In case of errors, store the error message in this buffer. The size of the buffer must be at least MAXRBUF.
     *  @return 0 on successful, -1 if there is an error and errmsg is set.
     */
    extern int IUReadConfig(const char *filename, const char *dev, const char *property, int silent, char errmsg[]);

    /** @brief Copies an existing configuration file into a default configuration file.
     *  If no <i>default</i> configuration file for the supplied <i>dev</i> exists, it gets created and its contentes copied from an exiting source configuration file.
     *  Usually, when the user saves the configuration file of a driver for the first time, IUSaveDefaultConfig is called to create the default
     *  configuration file. If the default configuration file already exists, the function performs no action and returns.
     *  @param source_config full path of the source configuration file to read. If set, the function will attempt to load the file.
     *         If set to NULL, it will attempt to generate the filename as described in the <b>Detailed Description</b> introduction and then load it.
     *  @param dest_config full path of the destination default configuration file to write. If set, the function will attempt to load the file.
     *         If set to NULL, it will attempt to generate the filename as described in the <b>Detailed Description</b> introduction and then load it.
     *         If the file already exists, the function returns. If the file doesn't exist, it gets created and its contents copied from the source_config file.
     *  @param dev device name. This is used if either the source_config or desg_config are NULL, and LITHIUMCONFIG environment variable is not set as described in the <b>Detailed Description</b> introduction.
     *  @return 0 is successful, -1 othereise.
     */
    extern int IUSaveDefaultConfig(const char *source_config, const char *dest_config, const char *dev);

    /** @brief Add opening or closing tag to a configuration file.
     *  A configuration file root XML element is \<LITHIUMDriver\>. This functions add \<LITHIUMDriver\> or \</LITHIUMDriver\> as required.
     *  @param fp file pointer to a configuration file.
     *  @param ctag If 0, \<LITHIUMDriver\> is appened to the configuration file, otherwise \</LITHIUMDriver\> is appeneded and the <i>fp</i> is closed.
     *  @param dev device name. Used only for sending notification to the driver if silent is set to 1.
     *  @param silent If silent is 1, it will suppress any output messages to the driver.
     */
    extern void IUSaveConfigTag(FILE *fp, int ctag, const char *dev, int silent);

    /** @brief IUGetConfigNumber Opens configuration file and reads single number property.
     *  @param dev name of device
     *  @param property name of vector property
     *  @param member name of member property
     *  @param value pointer to save value of property if found.
     *  @return 0 on success, -1 if not found.
     */
    extern int IUGetConfigNumber(const char *dev, const char *property, const char *member, double *value);

    /** @brief IUGetConfigSwitch Opens configuration file and reads single switch property.
     *  @param dev name of device
     *  @param property name of vector property
     *  @param member name of member property
     *  @param value pointer to save value of property if found.
     *  @return 0 on success, -1 if not found.
     */
    extern int IUGetConfigSwitch(const char *dev, const char *property, const char *member, ISState *value);

    /** @brief IUGetConfigOnSwitchIndex Opens configuration file and reads single switch property to find ON switch index, if any.
     *  @param dev name of device
     *  @param property name of vector property
     *  @param index pointer to save value of ON switch index, if found.
     *  @return 0 on success, -1 if not found.
     */
    extern int IUGetConfigOnSwitchIndex(const char *dev, const char *property, int *index);

    /** @brief IUGetConfigOnSwitchLabel Opens configuration file and returns the name of the ON switch property, if any.
     *  @param dev name of device
     *  @param property name of vector property
     *  @param name of the ON switch, if any.
     *  @param size size of name in bytes.
     *  @return 0 on success, -1 if not found.
     */
    extern int IUGetConfigOnSwitchName(const char *dev, const char *property, char *name, size_t size);

    /** @brief IUGetConfigOnSwitch Opens configuration file and reads a single switch vector property to find the index
     *  of the first ON switch element.
     *  @param property pointer to initialized switch vector property.
     *  @param index index of first ON switch property. If none is found, it is set to -1
     *  @return 0 on success, -1 if not found.
     */
    extern int IUGetConfigOnSwitch(const ISwitchVectorProperty *property, int *index);

    /** @brief IUGetConfigText Opens configuration file and reads single text property.
     *  @param dev name of device
     *  @param property name of vector property
     *  @param member name of member property
     *  @param value pointer to save value of property if found.
     *  @param len size of char buffer (value)
     *  @return 0 on success, -1 if not found.
     */
    extern int IUGetConfigText(const char *dev, const char *property, const char *member, char *value, int len);

    /** @brief Update all switches in a switch vector property.
     *  @param svp a pointer to a switch vector property.
     *  @param states the states of the new ISwitch members.
     *  @param names the names of the ISwtich members to update.
     *  @param n the number of ISwitch members to update.
     *  @return 0 if update successful, -1 otherwise.
     */
    extern int IUUpdateSwitch(ISwitchVectorProperty *svp, ISState *states, char *names[], int n);

    /** @brief Update all numbers in a number vector property.
     *  @param nvp a pointer to a number vector property.
     *  @param values the states of the new INumber members.
     *  @param names the names of the INumber members to update.
     *  @param n the number of INumber members to update.
     *  @return 0 if update successful, -1 otherwise. Update will fail if values are out of scope, or in case of property name mismatch.
     */
    extern int IUUpdateNumber(INumberVectorProperty *nvp, double values[], char *names[], int n);

    /** @brief Update all text members in a text vector property.
     *  @param tvp a pointer to a text vector property.
     *  @param texts a pointer to the text members
     *  @param names the names of the IText members to update.
     *  @param n the number of IText members to update.
     *  @return 0 if update successful, -1 otherwise. Update will fail in case of property name mismatch.
     */
    extern int IUUpdateText(ITextVectorProperty *tvp, char *texts[], char *names[], int n);

    /** @brief Update all BLOB members in a BLOB vector property.
     *  @param bvp a pointer to a BLOB vector property.
     *  @param sizes sizes of the blobs.
     *  @param blobsizes size of the blobs, raw without compression.
     *  @param blobs a pointer to the BLOB members
     *  @param names the names of the IBLOB members to update.
     *  @param formats The blob format or extension.
     *  @param n the number of IBLOB members to update.
     *  @return 0 if update successful, -1 otherwise. Update will fail in case of property name mismatch.
     */
    extern int IUUpdateBLOB(IBLOBVectorProperty *bvp, int sizes[], int blobsizes[], char *blobs[], char *formats[],
                            char *names[], int n);

    /** @brief Function to update the min and max elements of a number in the client
     *  @param nvp pointer to an INumberVectorProperty.
     *  @warning This call is not LITHIUM protocol compliant. It sends setNumberVector along with updated Min/Max/Step values so that the client
     *  updates the range accordingly for this property. In the LITHIUM-compliant paradigm, it is NOT possible to update min/max/step step of an existing number
     *  property and the only way is to do so is to delete and redefine the number property again. However, due to the many problems with approach in device drivers,
     *  LITHIUM Library defines this function to simplify the update process without requiring a complete delete/define cycle.
     */
    extern void IUUpdateMinMax(const INumberVectorProperty *nvp);

    /* @} */

#ifdef __cplusplus
}
#endif
