# mod_dpevents - FreeSWITCH module for events bindings from dialplan

Implements dialplan application **bind_event**.

Usage for **fs_cli**:

`originate {execute_on_originate='bind_event CHANNEL_HANGUP_COMPLETE log info TEST 1'}user/1000 &echo`

# Usage

Usage for **XML dialplan**:

`<action application="bind_event" data="CHANNEL_HANGUP_COMPLETE log info TEST 1"/>`

This will cause FreeSWITCH execute **log info TEST 1** when channel dies.

# Building

Module is belt as part of FreeSWITCH source tree.

To add **mod_dpevents** to build process follow next steps in FreeSWITCH source tree root:

+ `git branch devel-mod_dpevents`
+ `git checkout devel-mod_dpevents`
+ `git submodule add --name mod_dpevents https://github.com/kvishnivetsky/freeswitch-mod_dpevents.git src/mod/applications/mod_dpevents`
+ edit your `configure.ac` and add to `AC_CONFIG_FILES` section `src/mod/applications/mod_dpevents/Makefile`
+ edit your `modules.conf` and add `applications/mod_dpevents`
+ build FreeSWITCH as usual: `./bootstrap.sh` and so on....

# Legal information

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.
