Module dpevents for FreeSWITCH

Implements dialplan application bind_event.
Usage for fs_cli:
originate {execute_on_originate='bind_event CHANNEL_HANGUP_COMPLETE log info TEST 1'}user/1000 &echo

Usage for XML dialplan:
<action application="bind_event" data="CHANNEL_HANGUP_COMPLETE log info TEST 1"/>

This will cause FreeSWITCH execute log info TEST 1 when channel dies.
