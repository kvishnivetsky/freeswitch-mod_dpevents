Module dpevents for FreeSWITCH

Implements dialplan application bind_event.
Usage for fs_cli:
originate {execute_on_originate='bind_event CHANNEL_HANGUP_COMPLETE log info TEST 1'}user/1000 &echo

Usage for XML dialplan:
<action application="bind_event" data="CHANNEL_HANGUP_COMPLETE log info TEST 1"/>

This will cause FreeSWITCH execute log info TEST 1 when channel dies.

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.
