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

/** This files serves as an example how to use the C++ class (and also for me as a testbed)*/

#include <liblightify++/liblightify++.hpp>

#include <iostream>
#include <iomanip>

using namespace std;

static const char *decode_lamptype(int type) {
	switch (type) {
		case LIGHTIFY_ONOFF_PLUG:
			return "oo-plug";
		case LIGHTIFY_DIMABLE_LIGHT:
			return "dim";
		case LIGHTIFY_COLOUR_LIGHT:
			return "color";
		case LIGHTIFY_EXT_COLOUR_LIGHT:
			return "ext-col";
		case LIGHTIFY_CCT_LIGHT:
			return "cct";
		default:
			return "unknown";
	}
}

static const char *decode_onoff_sate(int state) {
	if (state < 0) return "err";
	switch (state) {
		case  0:
			return "off";
		case 1:
			 return "on";
		default:
			return "???";
	}
}

static const char *decode_online_state(int state) {
	switch (state) {
		case  LIGHTIFY_OFFLINE:
			return "offline";
		case LIGHTIFY_ONLINE:
			return "online";
		default:
			return "unknown";
	}
}


int main(void) {
	int err;

	Lightify l("lightify", 4000);
	err = l.Open();
	if (err < 0) cerr << "Lightify::Open failed: " << -err << " "  << strerror(-err) << endl;
	l.ScanNodes();

	Lightify_Node *node = 0;
	cout << "|-----------------|------------------|---------|-------|---------|-----|-----|------|-----|-----|-----|-----|---|" << endl;
	cout << "| Name            | MAC              | type    | group | online  | 0/1 | dim | CCT  | Red | Grn | Blu | Wht | s |" << endl;
	cout << "|-----------------|------------------|---------|-------|---------|-----|-----|------|-----|-----|-----|-----|---|" << endl;

	for(int i = 0; i< l.GetNodesCount(); i++) {
		node = l.GetNodeAtPosX(i);
		cout << '|'  <<
			setw(16) << node->GetName() << " | " <<
		    setw(16) << hex << node->GetMAC() << " | " <<
			setw(7)  << decode_lamptype(node->GetLampType()) << " | " <<
			setw(5)  << dec << node->GetGroup() << " | "  <<
			setw(7)  << decode_online_state(node->GetOnlineState()) << " | " <<
			setw(3)  << decode_onoff_sate(node->IsOn()) << " | "  <<
			setw(3)  << node->GetBrightness() << " | "  <<
			setw(4)  << node->GetCCT() << " | "  <<
			setw(3)  << node->GetRed() << " | "  <<
			setw(3)  << node->GetGreen() << " | "  <<
			setw(3)  << node->GetBlue() << " | "  <<
			setw(3)  << node->GetWhite() << " | "  <<
			setw(1) <<  (node->IsStale() ? '*' :' ') << " |" << endl;
	}


	cout << "|-----------------|------------------|---------|-------|---------|-----|-----|------|-----|-----|-----|-----|---|" << endl << endl;
	l.ScanGroups();
	cout << "|------------------|--------|" << endl;
	cout << "| Group Name       | id     |" << endl;
	cout << "|------------------|--------|" << endl;
	Lightify_Group *group = NULL;
	for (int i = 0; i < l.GetGroupsCount(); i++) {
		group = l.GetGroupAtPosX(i);
		cout << '|' <<
			setw(17) << group->GetName() << " | " <<
			setw(6)  << group->GetId() << " |" << endl;
	}
	cout  << "|------------------|--------|" << endl;

	return 0;
}
