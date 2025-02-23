# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the Google name nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import logging
import os
import signal
import subprocess

from webkitpy.layout_tests.models.test_configuration import TestConfiguration
from webkitpy.layout_tests.port.server_process import ServerProcess
from webkitpy.layout_tests.port.webkit import WebKitDriver, WebKitPort
from webkitpy.common.system.executive import Executive

_log = logging.getLogger(__name__)


class GtkDriver(WebKitDriver):
    def _start(self, pixel_tests, per_test_args):

        # Collect the number of X servers running already and make
        # sure our Xvfb process doesn't clash with any of them.
        def x_filter(process_name):
            return process_name.find("Xorg") > -1

        running_displays = len(Executive().running_pids(x_filter))

        # Use even displays for pixel tests and odd ones otherwise. When pixel tests are disabled,
        # DriverProxy creates two drivers, one for normal and the other for ref tests. Both have
        # the same worker number, so this prevents them from using the same Xvfb instance.
        display_id = self._worker_number * 2 + running_displays
        if self._port.get_option('pixel_tests'):
            display_id += 1
        run_xvfb = ["Xvfb", ":%d" % (display_id), "-screen",  "0", "800x600x24", "-nolisten", "tcp"]
        with open(os.devnull, 'w') as devnull:
            self._xvfb_process = subprocess.Popen(run_xvfb, stderr=devnull)
        server_name = self._port.driver_name()
        environment = self._port.setup_environ_for_server(server_name)
        # We must do this here because the DISPLAY number depends on _worker_number
        environment['DISPLAY'] = ":%d" % (display_id)
        self._crashed_process_name = None
        self._crashed_pid = None
        self._server_process = ServerProcess(self._port, server_name, self.cmd_line(pixel_tests, per_test_args), environment)

    def stop(self):
        WebKitDriver.stop(self)
        if getattr(self, '_xvfb_process', None):
            # FIXME: This should use Executive.kill_process
            os.kill(self._xvfb_process.pid, signal.SIGTERM)
            self._xvfb_process.wait()
            self._xvfb_process = None


class GtkPort(WebKitPort):
    port_name = "gtk"

    def _port_flag_for_scripts(self):
        return "--gtk"

    def _driver_class(self):
        return GtkDriver

    def _unload_pulseaudio_module(self):
        # Unload pulseaudio's module-stream-restore, since it remembers
        # volume settings from different runs, and could affect
        # multimedia tests results
        self._pa_module_index = -1
        with open(os.devnull, 'w') as devnull:
            try:
                pactl_process = subprocess.Popen(["pactl", "list", "short", "modules"], stdout=subprocess.PIPE, stderr=devnull)
                pactl_process.wait()
            except OSError:
                # pactl might not be available.
                _log.debug('pactl not found. Please install pulseaudio-utils to avoid some potential media test failures.')
                return
        modules_list = pactl_process.communicate()[0]
        for module in modules_list.splitlines():
            if module.find("module-stream-restore") >= 0:
                # Some pulseaudio-utils versions don't provide
                # the index, just an empty string
                self._pa_module_index = module.split('\t')[0] or -1
                try:
                    # Since they could provide other stuff (not an index
                    # nor an empty string, let's make sure this is an int.
                    if int(self._pa_module_index) != -1:
                        pactl_process = subprocess.Popen(["pactl", "unload-module", self._pa_module_index])
                        pactl_process.wait()
                        if pactl_process.returncode == 0:
                            _log.debug('Unloaded module-stream-restore successfully')
                        else:
                            _log.debug('Unloading module-stream-restore failed')
                except ValueError:
                        # pactl should have returned an index if the module is found
                        _log.debug('Unable to parse module index. Please check if your pulseaudio-utils version is too old.')
                return

    def _restore_pulseaudio_module(self):
        # If pulseaudio's module-stream-restore was previously unloaded,
        # restore it back. We shouldn't need extra checks here, since an
        # index != -1 here means we successfully unloaded it previously.
        if self._pa_module_index != -1:
            with open(os.devnull, 'w') as devnull:
                pactl_process = subprocess.Popen(["pactl", "load-module", "module-stream-restore"], stdout=devnull, stderr=devnull)
                pactl_process.wait()
                if pactl_process.returncode == 0:
                    _log.debug('Restored module-stream-restore successfully')
                else:
                    _log.debug('Restoring module-stream-restore failed')


    def setup_test_run(self):
        self._unload_pulseaudio_module()

    def clean_up_test_run(self):
        self._restore_pulseaudio_module()

    def setup_environ_for_server(self, server_name=None):
        environment = WebKitPort.setup_environ_for_server(self, server_name)
        environment['GTK_MODULES'] = 'gail'
        environment['GSETTINGS_BACKEND'] = 'memory'
        environment['LIBOVERLAY_SCROLLBAR'] = '0'
        environment['TEST_RUNNER_INJECTED_BUNDLE_FILENAME'] = self._build_path('Libraries', 'libTestRunnerInjectedBundle.la')
        environment['TEST_RUNNER_TEST_PLUGIN_PATH'] = self._build_path('TestNetscapePlugin', '.libs')
        environment['WEBKIT_INSPECTOR_PATH'] = self._build_path('Programs', 'resources', 'inspector')
        environment['AUDIO_RESOURCES_PATH'] = self._filesystem.join(self._config.webkit_base_dir(),
                                                                    'Source', 'WebCore', 'platform',
                                                                    'audio', 'resources')
        if self.get_option('webkit_test_runner'):
            # FIXME: This is a workaround to ensure that testing with WebKitTestRunner is started with
            # a non-existing cache. This should be removed when (and if) it will be possible to properly
            # set the cache directory path through a WebKitWebContext.
            environment['XDG_CACHE_HOME'] = self._filesystem.join(self.results_directory(), 'appcache')
        return environment

    def _generate_all_test_configurations(self):
        configurations = []
        for build_type in self.ALL_BUILD_TYPES:
            configurations.append(TestConfiguration(version=self._version, architecture='x86', build_type=build_type))
        return configurations

    def _path_to_driver(self):
        return self._build_path('Programs', self.driver_name())

    def _path_to_image_diff(self):
        return self._build_path('Programs', 'ImageDiff')

    def check_build(self, needs_http):
        return self._check_driver()

    def _path_to_apache(self):
        if self._is_redhat_based():
            return '/usr/sbin/httpd'
        else:
            return '/usr/sbin/apache2'

    def _path_to_wdiff(self):
        if self._is_redhat_based():
            return '/usr/bin/dwdiff'
        else:
            return '/usr/bin/wdiff'

    def _path_to_webcore_library(self):
        gtk_library_names = [
            "libwebkitgtk-1.0.so",
            "libwebkitgtk-3.0.so",
            "libwebkit2gtk-1.0.so",
        ]

        for library in gtk_library_names:
            full_library = self._build_path(".libs", library)
            if self._filesystem.isfile(full_library):
                return full_library
        return None

    # FIXME: We should find a way to share this implmentation with Gtk,
    # or teach run-launcher how to call run-safari and move this down to WebKitPort.
    def show_results_html_file(self, results_filename):
        run_launcher_args = ["file://%s" % results_filename]
        if self.get_option('webkit_test_runner'):
            run_launcher_args.append('-2')
        # FIXME: old-run-webkit-tests also added ["-graphicssystem", "raster", "-style", "windows"]
        # FIXME: old-run-webkit-tests converted results_filename path for cygwin.
        self._run_script("run-launcher", run_launcher_args)

    def _get_gdb_output(self, coredump_path):
        cmd = ['gdb', '-ex', 'thread apply all bt', '--batch', str(self._path_to_driver()), coredump_path]
        proc = subprocess.Popen(cmd, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        proc.wait()
        errors = [l.strip() for l in proc.stderr.readlines()]
        return (proc.stdout.read(), errors)

    def _get_crash_log(self, name, pid, stdout, stderr, newer_than):
        pid_representation = str(pid or '<unknown>')
        log_directory = os.environ.get("WEBKIT_CORE_DUMPS_DIRECTORY")
        errors = []
        crash_log = ''
        expected_crash_dump_filename = "core-pid_%s-_-process_%s" % (pid_representation, name)

        def match_filename(filesystem, directory, filename):
            if pid:
                return filename == expected_crash_dump_filename
            return filename.find(name) > -1

        if log_directory:
            dumps = self._filesystem.files_under(log_directory, file_filter=match_filename)
            if dumps:
                # Get the most recent coredump matching the pid and/or process name.
                coredump_path = list(reversed(sorted(dumps)))[0]
                if not newer_than or self._filesystem.mtime(coredump_path) > newer_than:
                    crash_log, errors = self._get_gdb_output(coredump_path)

        stderr_lines = errors + (stderr or '<empty>').decode('utf8', 'ignore').splitlines()
        errors_str = '\n'.join(('STDERR: ' + l) for l in stderr_lines)
        if not crash_log:
            if not log_directory:
                log_directory = "/path/to/coredumps"
            core_pattern = os.path.join(log_directory, "core-pid_%p-_-process_%e")
            crash_log = """\
Coredump %(expected_crash_dump_filename)s not found. To enable crash logs:

- run this command as super-user: echo "%(core_pattern)s" > /proc/sys/kernel/core_pattern
- enable core dumps: ulimit -c unlimited
- set the WEBKIT_CORE_DUMPS_DIRECTORY environment variable: export WEBKIT_CORE_DUMPS_DIRECTORY=%(log_directory)s

""" % locals()

        return """\
Crash log for %(name)s (pid %(pid_representation)s):

%(crash_log)s
%(errors_str)s""" % locals()
