/*
  liblightify -- library to control OSRAM's LIGHTIFY

Copyright (c) 2015, Tobias Frost <tobi@coldtobi.de>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * \file groups.h
 *
 *  Created on: 18.11.2015
 *      Author: tobi
 */

#ifndef SRC_GROUPS_H_
#define SRC_GROUPS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** Generate a new group object
 *
 * @param ctx  Library context
 * @param newgroup where to store new pointer of new group
 * @return negative on error. >=0 is success.
 */
int lightify_group_new(struct lightify_ctx *ctx, struct lightify_group **newgroup);

/** Set Group's name
 *
 * @param grp to operate on
 * @param name to set
 * @return negative on error. >=0 is success.
 *
 * \note a maximum lenght of 16 chars is enforced.
 * \note a copy of the name is allocated and stored.
 */
int lightify_group_set_name(struct lightify_group *grp, const unsigned char *name);

/** Set Group's ID
 *
 * @param grp to operate on
 * @param id to set
 * @return negative on error. >=0 is success.
 */
int lightify_group_set_id(struct lightify_group *grp, int id);

/** Remove group from linked list and free memory associated.
 *
 * @param grp to operate on
 * @return negative on error. >=0 is success.
 */
int lightify_group_remove(struct lightify_group *grp);

#endif /* SRC_GROUPS_H_ */
