/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#ifndef VRN_VOLUMEHANDLEVALIDATOR_H
#define VRN_VOLUMEHANDLEVALIDATOR_H

namespace voreen {

class VolumeHandle;

/**
 * Class used for validating VolumeHandles.
 * Whenever a processor uses its LocalPortMapping structure within process() to
 * use a VolumeHandle* from it, one or more of the following things might happen:
 *
 * - the VolumeHandle* is NULL, so rendering usually cannot be continued
 * - the VolumeHandle* is different from the previously used one
 * - the VolumeHandle* is the same as the previously used one
 *
 * In order to check whether the VolumeHandle* obtained from the LocalPortMapping
 * and in order to maybe assign it to an attribute within the processor,
 * checkVolumeHandle() from this class can be used. Optionally it is checked whether
 * the VolumeHandle* has changed and / or whether it contains a non-NULL VolumeGL*.
 *
 * Further useful checking method could be integrated here.
 *
 * <b>NOTE:</b>
 * This class acts as a static one. You can neither instantiate it nor copy objects
 * from it as the ctors and operator =() are declared as private as you will see.
 * Do not inherit from this class either!
 *
 * @author  Dirk Feldmann, March 2009 A.D.
 *
 */
class VolumeHandleValidator {
public:
    /**
     * Checks whether the given <code>VolumeHandle* newHandle</code> is valid and sets it
     * as to the one given in <code>handle</code> if so. If you specify the third parameter,
     * the passed bool indicates whether the volume handle has been changed or not after the
     * methods returns. You can use this information to optimize rendering performance.
     *
     * Call this method from <code>process()</code> with <code>newHandle</code> set to the
     * one from the given <code>LocalPortMapping</code> object and with <code>handle</code>
     * set to the address of the <code>VolumeHandle*</code> frm the processor you wish to
     * assign it to. This will ensure that the processor has got a valid volume handle and
     * may proceed rendering.
     *
     * @param   handle      The address of the <code>VolumeHandle*</code> from the processors
     *                      which shall be assigned.
     * @param   newHandle       The <code>VolumeHandle</code> which shall be set as the current one
     * @param   handleChanged   If you pass an address of bool, its value will be set to
     *                          "true" if the current handle has been changed, even if it
     *                          was set to NULL!
     * @param   omitVolumeCheck Influences the returned value:
     *                          If this parameter is set to "true", it is only checked whether
     *                          the volume handle is valid. Otherwise it will also be checked
     *                          if the contained volume is valid, too.
     *
     * @return  If omitVolumeCheck was set to "false", the return value is "true"
     *          if currentVolumeHandle_ is <b>not NULL AND the contained
     *          volume within is not NULL either</b> or "false" otherwise.
     *          If omitVolumeCheck was set to "false", the return value is "true"
     *          if only currentVolumeHandle_ is <b>not NULL</b> or "false" otherwise
     *
     */
    static bool checkVolumeHandle(VolumeHandle*& handle, VolumeHandle* const newHandle,
                                  bool* handleChanged = 0, const bool omitVolumeCheck = false);

    // prevent creating and copying of objects of VolumeHandleValidator by
    // declaring ctors and assignment operator as private so that compiler
    // does not provide public ones.
    //
private:
    VolumeHandleValidator();
    VolumeHandleValidator(const VolumeHandleValidator&);
    VolumeHandleValidator& operator =(const VolumeHandleValidator&);
};

} // namespace

#endif
