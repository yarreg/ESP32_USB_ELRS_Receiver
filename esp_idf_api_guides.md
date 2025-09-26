This file is a merged representation of a subset of the codebase, containing specifically included files, combined into a single document by Repomix.

# File Summary

## Purpose
This file contains a packed representation of a subset of the repository's contents that is considered the most important context.
It is designed to be easily consumable by AI systems for analysis, code review,
or other automated processes.

## File Format
The content is organized as follows:
1. This summary section
2. Repository information
3. Directory structure
4. Repository files (if enabled)
5. Multiple file entries, each consisting of:
  a. A header with the file path (## File: path/to/file)
  b. The full contents of the file in a code block

## Usage Guidelines
- This file should be treated as read-only. Any changes should be made to the
  original repository files, not this packed version.
- When processing this file, use the file path to distinguish
  between different files in the repository.
- Be aware that this file may contain sensitive information. Handle it with
  the same level of security as you would the original repository.

## Notes
- Some files may have been excluded based on .gitignore rules and Repomix's configuration
- Binary files are not included in this packed representation. Please refer to the Repository Structure section for a complete list of file paths, including binary files
- Only files matching these patterns are included: docs/en/api-guides/*.rst
- Files matching patterns in .gitignore are excluded
- Files matching default ignore patterns are excluded
- Files are sorted by Git change count (files with more changes are at the bottom)

# Directory Structure
```
docs/en/api-guides/app_trace.rst
docs/en/api-guides/bootloader.rst
docs/en/api-guides/build-system.rst
docs/en/api-guides/c.rst
docs/en/api-guides/coexist.rst
docs/en/api-guides/core_dump_internals.rst
docs/en/api-guides/core_dump.rst
docs/en/api-guides/cplusplus.rst
docs/en/api-guides/current-consumption-measurement-modules.rst
docs/en/api-guides/deep-sleep-stub.rst
docs/en/api-guides/dfu.rst
docs/en/api-guides/error-handling.rst
docs/en/api-guides/esp-wifi-mesh.rst
docs/en/api-guides/external-ram.rst
docs/en/api-guides/fatal-errors.rst
docs/en/api-guides/file-system-considerations.rst
docs/en/api-guides/flash_psram_config.rst
docs/en/api-guides/general-notes.rst
docs/en/api-guides/hardware-abstraction.rst
docs/en/api-guides/hlinterrupts.rst
docs/en/api-guides/host-apps.rst
docs/en/api-guides/index.rst
docs/en/api-guides/linker-script-generation.rst
docs/en/api-guides/lwip.rst
docs/en/api-guides/memory-types.rst
docs/en/api-guides/openthread.rst
docs/en/api-guides/partition-tables.rst
docs/en/api-guides/phy.rst
docs/en/api-guides/reproducible-builds.rst
docs/en/api-guides/RF_calibration.rst
docs/en/api-guides/startup.rst
docs/en/api-guides/stdio.rst
docs/en/api-guides/thread-local-storage.rst
docs/en/api-guides/unit-tests.rst
docs/en/api-guides/usb-otg-console.rst
docs/en/api-guides/usb-serial-jtag-console.rst
docs/en/api-guides/wifi-expansion.rst
docs/en/api-guides/wifi-security.rst
docs/en/api-guides/wifi.rst
docs/en/api-guides/wireshark-user-guide.rst
```

# Files

## File: docs/en/api-guides/app_trace.rst
````
Application Level Tracing Library
=================================

:link_to_translation:`zh_CN:[中文]`

Overview
--------

ESP-IDF provides a useful feature for program behavior analysis: application level tracing. It is implemented in the corresponding library and can be enabled in menuconfig. This feature allows to transfer arbitrary data between host and {IDF_TARGET_NAME} via JTAG, UART, or USB interfaces with small overhead on program execution. It is possible to use JTAG and UART interfaces simultaneously. The UART interface is mostly used for connection with SEGGER SystemView tool (see `SystemView <https://www.segger.com/products/development-tools/systemview/>`_).

Developers can use this library to send application-specific state of execution to the host and receive commands or other types of information from the opposite direction at runtime. The main use cases of this library are:

1. Collecting application-specific data. See :ref:`app_trace-application-specific-tracing`.
2. Lightweight logging to the host. See :ref:`app_trace-logging-to-host`.
3. System behavior analysis. See :ref:`app_trace-system-behaviour-analysis-with-segger-systemview`.
4. Source code coverage. See :ref:`app_trace-gcov-source-code-coverage`.

Tracing components used when working over JTAG interface are shown in the figure below.

.. figure:: ../../_static/app_trace-overview.jpg
    :align: center
    :alt: Tracing Components When Working Over JTAG

    Tracing Components Used When Working Over JTAG


Modes of Operation
------------------

The library supports two modes of operation:

**Post-mortem mode:** This is the default mode. The mode does not need interaction with the host side. In this mode, tracing module does not check whether the host has read all the data from *HW UP BUFFER*, but directly overwrites old data with the new ones. This mode is useful when only the latest trace data is interesting to the user, e.g., for analyzing program's behavior just before the crash. The host can read the data later on upon user request, e.g., via special OpenOCD command in case of working via JTAG interface.

**Streaming mode:** Tracing module enters this mode when the host connects to {IDF_TARGET_NAME}. In this mode, before writing new data to *HW UP BUFFER*, the tracing module checks that whether there is enough space in it and if necessary, waits for the host to read data and free enough memory. Maximum waiting time is controlled via timeout values passed by users to corresponding API routines. So when application tries to write data to the trace buffer using the finite value of the maximum waiting time, it is possible that this data will be dropped. This is especially true for tracing from time critical code (ISRs, OS scheduler code, etc.) where infinite timeouts can lead to system malfunction.


Configuration Options and Dependencies
--------------------------------------

Using of this feature depends on two components:

1. **Host side:** Application tracing is done over JTAG, so it needs OpenOCD to be set up and running on host machine. For instructions on how to set it up, please see :doc:`JTAG Debugging <../api-guides/jtag-debugging/index>` for details.

2. **Target side:** Application tracing functionality can be enabled in menuconfig. Please go to ``Component config`` > ``Application Level Tracing`` menu, which allows selecting destination for the trace data (hardware interface for transport: JTAG or/and UART). Choosing any of the destinations automatically enables the ``CONFIG_APPTRACE_ENABLE`` option. For UART interfaces, users have to define port number, baud rate, TX and RX pins numbers, and additional UART-related parameters.

.. note::

    In order to achieve higher data rates and minimize the number of dropped packets, it is recommended to optimize the setting of JTAG clock frequency, so that it is at maximum and still provides stable operation of JTAG. See :ref:`jtag-debugging-tip-optimize-jtag-speed`.

There are two additional menuconfig options not mentioned above:

1. *Threshold for flushing last trace data to host on panic* (:ref:`CONFIG_APPTRACE_POSTMORTEM_FLUSH_THRESH`). This option is necessary due to the nature of working over JTAG. In this mode, trace data is exposed to the host in 16 KB blocks. In post-mortem mode, when one block is filled, it is exposed to the host and the previous one becomes unavailable. In other words, the trace data is overwritten in 16 KB granularity. On panic, the latest data from the current input block is exposed to the host and the host can read them for post-analysis. System panic may occur when a very small amount of data are not exposed to the host yet. In this case, the previous 16 KB of collected data will be lost and the host will see the latest, but very small piece of the trace. It can be insufficient to diagnose the problem. This menuconfig option allows avoiding such situations. It controls the threshold for flushing data in case of apanic. For example, users can decide that it needs no less than 512 bytes of the recent trace data, so if there is less then 512 bytes of pending data at the moment of panic, they will not be flushed and will not overwrite the previous 16 KB. The option is only meaningful in post-mortem mode and when working over JTAG.

2. *Timeout for flushing last trace data to host on panic* (:ref:`CONFIG_APPTRACE_ONPANIC_HOST_FLUSH_TMO`). The option is only meaningful in streaming mode and it controls the maximum time that the tracing module will wait for the host to read the last data in case of panic.

3. *UART RX/TX ring buffer size* (:ref:`CONFIG_APPTRACE_UART_TX_BUFF_SIZE`). The size of the buffer depends on the amount of data transferred through the UART.

4. *UART TX message size* (:ref:`CONFIG_APPTRACE_UART_TX_MSG_SIZE`). The maximum size of the single message to transfer.


How to Use This Library
-----------------------

This library provides APIs for transferring arbitrary data between the host and {IDF_TARGET_NAME}. When enabled in menuconfig, the target application tracing module is initialized automatically at the system startup, so all what the user needs to do is to call corresponding APIs to send, receive or flush the data.


.. _app_trace-application-specific-tracing:

Application Specific Tracing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In general, users should decide what type of data should be transferred in every direction and how these data must be interpreted (processed). The following steps must be performed to transfer data between the target and the host:

1. On the target side, users should implement algorithms for writing trace data to the host. Piece of code below shows an example on how to do this.

    .. code-block:: c

        #include "esp_app_trace.h"
        ...
        char buf[] = "Hello World!";
        esp_err_t res = esp_apptrace_write(ESP_APPTRACE_DEST_JTAG, buf, strlen(buf), ESP_APPTRACE_TMO_INFINITE);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write data to host!");
            return res;
        }

    ``esp_apptrace_write()`` function uses memcpy to copy user data to the internal buffer. In some cases, it can be more optimal to use ``esp_apptrace_buffer_get()`` and ``esp_apptrace_buffer_put()`` functions. They allow developers to allocate buffer and fill it themselves. The following piece of code shows how to do this.

    .. code-block:: c

        #include "esp_app_trace.h"
        ...
        int number = 10;
        char *ptr = (char *)esp_apptrace_buffer_get(ESP_APPTRACE_DEST_JTAG, 32, 100/*tmo in us*/);
        if (ptr == NULL) {
            ESP_LOGE(TAG, "Failed to get buffer!");
            return ESP_FAIL;
        }
        sprintf(ptr, "Here is the number %d", number);
        esp_err_t res = esp_apptrace_buffer_put(ESP_APPTRACE_DEST_JTAG, ptr, 100/*tmo in us*/);
        if (res != ESP_OK) {
            /* in case of error host tracing tool (e.g., OpenOCD) will report incomplete user buffer */
            ESP_LOGE(TAG, "Failed to put buffer!");
            return res;
        }

    Also according to his needs, the user may want to receive data from the host. Piece of code below shows an example on how to do this.

    .. code-block:: c

        #include "esp_app_trace.h"
        ...
        char buf[32];
        char down_buf[32];
        size_t sz = sizeof(buf);

        /* config down buffer */
        esp_err_t res = esp_apptrace_down_buffer_config(ESP_APPTRACE_DEST_JTAG, down_buf, sizeof(down_buf));
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Failed to config down buffer!");
            return res;
        }
        /* check for incoming data and read them if any */
        res = esp_apptrace_read(ESP_APPTRACE_DEST_JTAG, buf, &sz, 0/*do not wait*/);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read data from host!");
            return res;
        }
        if (sz > 0) {
            /* we have data, process them */
            ...
        }

    ``esp_apptrace_read()`` function uses memcpy to copy host data to user buffer. In some casesm it can be more optimal to use ``esp_apptrace_down_buffer_get()`` and ``esp_apptrace_down_buffer_put()`` functions. They allow developers to occupy chunk of read buffer and process it in-place. The following piece of code shows how to do this.

    .. code-block:: c

        #include "esp_app_trace.h"
        ...
        char down_buf[32];
        uint32_t *number;
        size_t sz = 32;

        /* config down buffer */
        esp_err_t res = esp_apptrace_down_buffer_config(ESP_APPTRACE_DEST_JTAG, down_buf, sizeof(down_buf));
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "Failed to config down buffer!");
            return res;
        }
        char *ptr = (char *)esp_apptrace_down_buffer_get(ESP_APPTRACE_DEST_JTAG, &sz, 100/*tmo in us*/);
        if (ptr == NULL) {
            ESP_LOGE(TAG, "Failed to get buffer!");
            return ESP_FAIL;
        }
        if (sz > 4) {
            number = (uint32_t *)ptr;
            printf("Here is the number %d", *number);
        } else {
            printf("No data");
        }
        res = esp_apptrace_down_buffer_put(ESP_APPTRACE_DEST_JTAG, ptr, 100/*tmo in us*/);
        if (res != ESP_OK) {
            /* in case of error host tracing tool (e.g., OpenOCD) will report incomplete user buffer */
            ESP_LOGE(TAG, "Failed to put buffer!");
            return res;
        }

2. The next step is to build the program image and download it to the target as described in the :ref:`Getting Started Guide <get-started-build>`.

3. Run OpenOCD (see :doc:`JTAG Debugging <../api-guides/jtag-debugging/index>`).

4. Connect to OpenOCD telnet server. It can be done using the following command in terminal ``telnet <oocd_host> 4444``. If telnet session is opened on the same machine which runs OpenOCD, you can use ``localhost`` as ``<oocd_host>`` in the command above.

5. Start trace data collection using special OpenOCD command. This command will transfer tracing data and redirect them to the specified file or socket (currently only files are supported as trace data destination). For description of the corresponding commands, see `OpenOCD Application Level Tracing Commands`_.

6. The final step is to process received data. Since the format of data is defined by users, the processing stage is out of the scope of this document. Good starting points for data processor are python scripts in ``$IDF_PATH/tools/esp_app_trace``: ``apptrace_proc.py`` (used for feature tests) and ``logtrace_proc.py`` (see more details in section `Logging to Host`_).


OpenOCD Application Level Tracing Commands
""""""""""""""""""""""""""""""""""""""""""

*HW UP BUFFER* is shared between user data blocks and the filling of the allocated memory is performed on behalf of the API caller (in task or ISR context). In multithreading environment, it can happen that the task/ISR which fills the buffer is preempted by another high priority task/ISR. So it is possible that the user data preparation process is not completed at the moment when that chunk is read by the host. To handle such conditions, the tracing module prepends all user data chunks with header which contains the allocated user buffer size (2 bytes) and the length of the actually written data (2 bytes). So the total length of the header is 4 bytes. OpenOCD command which reads trace data reports error when it reads incomplete user data chunk, but in any case, it puts the contents of the whole user chunk (including unfilled area) to the output file.

Below is the description of available OpenOCD application tracing commands.

.. note::

    Currently, OpenOCD does not provide commands to send arbitrary user data to the target.


Command usage:

``esp apptrace [start <options>] | [stop] | [status] | [dump <cores_num> <outfile>]``

Sub-commands:

``start``
    Start tracing (continuous streaming).
``stop``
    Stop tracing.
``status``
    Get tracing status.
``dump``
    Dump all data from  (post-mortem dump).


Start command syntax:

  ``start <outfile> [poll_period [trace_size [stop_tmo [wait4halt [skip_size]]]]``

``outfile``
    Path to file to save data from both CPUs. This argument should have the following format: ``file://path/to/file``.
``poll_period``
    Data polling period (in ms) for available trace data. If greater than 0, then command runs in non-blocking mode. By default, 1 ms.
``trace_size``
    Maximum size of data to collect (in bytes). Tracing is stopped after specified amount of data is received. By default, -1 (trace size stop trigger is disabled).
``stop_tmo``
    Idle timeout (in sec). Tracing is stopped if there is no data for specified period of time. By default, -1 (disable this stop trigger). Optionally set it to value longer than longest pause between tracing commands from target.
``wait4halt``
    If 0, start tracing immediately, otherwise command waits for the target to be halted (after reset, by breakpoint etc.) and then automatically resumes it and starts tracing. By default, 0.
``skip_size``
    Number of bytes to skip at the start. By default, 0.

.. note::

    If ``poll_period`` is 0, OpenOCD telnet command line will not be available until tracing is stopped. You must stop it manually by resetting the board or pressing Ctrl+C in OpenOCD window (not one with the telnet session). Another option is to set ``trace_size`` and wait until this size of data is collected. At this point, tracing stops automatically.

Command usage examples:

.. highlight:: none

1. Collect 2048 bytes of tracing data to the file ``trace.log``. The file will be saved in the ``openocd-esp32`` directory.

    ::

        esp apptrace start file://trace.log 1 2048 5 0 0

    The tracing data will be retrieved and saved in non-blocking mode. This process will stop automatically after 2048 bytes are collected, or if no data are available for more than 5 seconds.

    .. note::

        Tracing data is buffered before it is made available to OpenOCD. If you see "Data timeout!" message, then it is likely that the target is not sending enough data to empty the buffer to OpenOCD before the timeout. Either increase the timeout or use the function ``esp_apptrace_flush()`` to flush the data on specific intervals.

2.  Retrieve tracing data indefinitely in non-blocking mode.

    ::

        esp apptrace start file://trace.log 1 -1 -1 0 0

    There is no limitation on the size of collected data and there is no data timeout set. This process may be stopped by issuing ``esp apptrace stop`` command on OpenOCD telnet prompt, or by pressing Ctrl+C in OpenOCD window.

3.  Retrieve tracing data and save them indefinitely.

    ::

        esp apptrace start file://trace.log 0 -1 -1 0 0

    OpenOCD telnet command line prompt will not be available until tracing is stopped. To stop tracing, press Ctrl+C in the OpenOCD window.

4.  Wait for the target to be halted. Then resume the target's operation and start data retrieval. Stop after collecting 2048 bytes of data:

    ::

        esp apptrace start file://trace.log 0 2048 -1 1 0

    To configure tracing immediately after reset, use the OpenOCD ``reset halt`` command.


.. _app_trace-logging-to-host:

Logging to Host
^^^^^^^^^^^^^^^

ESP-IDF implements a useful feature: logging to the host via application level tracing library. This is a kind of semihosting when all `ESP_LOGx` calls send strings to be printed to the host instead of UART. This can be useful because "printing to host" eliminates some steps performed when logging to UART. Most part of the work is done on the host.

By default, ESP-IDF's logging library uses vprintf-like function to write formatted output to dedicated UART. In general, it involves the following steps:

1. Format string is parsed to obtain type of each argument.
2. According to its type, every argument is converted to string representation.
3. Format string combined with converted arguments is sent to UART.

Though the implementation of the vprintf-like function can be optimized to a certain level, all steps above have to be performed in any case and every step takes some time (especially item 3). So it frequently occurs that with additional log added to the program to identify the problem, the program behavior is changed and the problem cannot be reproduced. And in the worst cases, the program cannot work normally at all and ends up with an error or even hangs.

Possible ways to overcome this problem are to use higher UART bitrates (or another faster interface) and/or to move string formatting procedure to the host.

The application level tracing feature can be used to transfer log information to the host using ``esp_apptrace_vprintf`` function. This function does not perform full parsing of the format string and arguments. Instead, it just calculates the number of arguments passed and sends them along with the format string address to the host. On the host, log data is processed and printed out by a special Python script.


Limitations
"""""""""""

Current implementation of logging over JTAG has some limitations:

1. No support for tracing from ``ESP_EARLY_LOGx`` macros.
2. No support for printf arguments whose size exceeds 4 bytes (e.g., ``double`` and ``uint64_t``).
3. Only strings from the .rodata section are supported as format strings and arguments.
4. The maximum number of printf arguments is 256.


How To Use It
"""""""""""""

In order to use logging via trace module, users need to perform the following steps:

1. On the target side, the special vprintf-like function :cpp:func:`esp_apptrace_vprintf` needs to be installed. It sends log data to the host. An example is ``esp_log_set_vprintf(esp_apptrace_vprintf);``. To send log data to UART again, use ``esp_log_set_vprintf(vprintf);``.
2. Follow instructions in items 2-5 in `Application Specific Tracing`_.
3. To print out collected log records, run the following command in terminal: ``$IDF_PATH/tools/esp_app_trace/logtrace_proc.py /path/to/trace/file /path/to/program/elf/file``.


Log Trace Processor Command Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Command usage:

``logtrace_proc.py [-h] [--no-errors] <trace_file> <elf_file>``

Positional arguments:

``trace_file``
    Path to log trace file.
``elf_file``
    Path to program ELF file.

Optional arguments:

``-h``, ``--help``
    Show this help message and exit.
``--no-errors``, ``-n``
    Do not print errors.


.. _app_trace-system-behaviour-analysis-with-segger-systemview:

System Behavior Analysis with SEGGER SystemView
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Another useful ESP-IDF feature built on top of application tracing library is the system level tracing which produces traces compatible with SEGGER SystemView tool (see `SystemView <https://www.segger.com/products/development-tools/systemview/>`_). SEGGER SystemView is a real-time recording and visualization tool that allows to analyze runtime behavior of an application. It is possible to view events in real-time through the UART interface.


How To Use It
"""""""""""""

Support for this feature is enabled by ``Component config`` > ``Application Level Tracing`` > ``FreeRTOS SystemView Tracing`` (:ref:`CONFIG_APPTRACE_SV_ENABLE`) menuconfig option. There are several other options enabled under the same menu:

1. SytemView destination. Select the destination interface: JTAG or UART. In case of UART, it will be possible to connect SystemView application to the {IDF_TARGET_NAME} directly and receive data in real-time.

2. {IDF_TARGET_NAME} timer to use as SystemView timestamp source: (:ref:`CONFIG_APPTRACE_SV_TS_SOURCE`) selects the source of timestamps for SystemView events. In the single core mode, timestamps are generated using {IDF_TARGET_NAME} internal cycle counter running at maximum 240 Mhz (about 4 ns granularity). In the dual-core mode, external timer working at 40 Mhz is used, so the timestamp granularity is 25 ns.

3. Individually enabled or disabled collection of SystemView events (``CONFIG_APPTRACE_SV_EVT_XXX``):

    - Trace Buffer Overflow Event
    - ISR Enter Event
    - ISR Exit Event
    - ISR Exit to Scheduler Event
    - Task Start Execution Event
    - Task Stop Execution Event
    - Task Start Ready State Event
    - Task Stop Ready State Event
    - Task Create Event
    - Task Terminate Event
    - System Idle Event
    - Timer Enter Event
    - Timer Exit Event

ESP-IDF has all the code required to produce SystemView compatible traces, so users can just configure necessary project options (see above), build, download the image to target, and use OpenOCD to collect data as described in the previous sections.

4. Select Pro or App CPU in menuconfig options ``Component config`` > ``Application Level Tracing`` > ``FreeRTOS SystemView Tracing`` to trace over the UART interface in real-time.


OpenOCD SystemView Tracing Command Options
""""""""""""""""""""""""""""""""""""""""""

Command usage:

``esp sysview [start <options>] | [stop] | [status]``

Sub-commands:

``start``
    Start tracing (continuous streaming).
``stop``
    Stop tracing.
``status``
    Get tracing status.

Start command syntax:

  ``start <outfile1> [outfile2] [poll_period [trace_size [stop_tmo]]]``

``outfile1``
    Path to file to save data from PRO CPU. This argument should have the following format: ``file://path/to/file``.
``outfile2``
    Path to file to save data from APP CPU. This argument should have the following format: ``file://path/to/file``.
``poll_period``
    Data polling period (in ms) for available trace data. If greater than 0, then command runs in non-blocking mode. By default, 1 ms.
``trace_size``
    Maximum size of data to collect (in bytes). Tracing is stopped after specified amount of data is received. By default, -1 (trace size stop trigger is disabled).
``stop_tmo``
    Idle timeout (in sec). Tracing is stopped if there is no data for specified period of time. By default, -1 (disable this stop trigger).

.. note::

    If ``poll_period`` is 0, OpenOCD telnet command line will not be available until tracing is stopped. You must stop it manually by resetting the board or pressing Ctrl+C in the OpenOCD window (not the one with the telnet session). Another option is to set ``trace_size`` and wait until this size of data is collected. At this point, tracing stops automatically.

Command usage examples:

.. highlight:: none

1.  Collect SystemView tracing data to files ``pro-cpu.SVDat`` and ``app-cpu.SVDat``. The files will be saved in ``openocd-esp32`` directory.

    ::

        esp sysview start file://pro-cpu.SVDat file://app-cpu.SVDat

    The tracing data will be retrieved and saved in non-blocking mode. To stop this process, enter ``esp sysview stop`` command on OpenOCD telnet prompt, optionally pressing Ctrl+C in the OpenOCD window.

2.  Retrieve tracing data and save them indefinitely.

    ::

        esp sysview start file://pro-cpu.SVDat file://app-cpu.SVDat 0 -1 -1

    OpenOCD telnet command line prompt will not be available until tracing is stopped. To stop tracing, press Ctrl+C in the OpenOCD window.


Data Visualization
""""""""""""""""""

After trace data are collected, users can use a special tool to visualize the results and inspect behavior of the program.

.. only:: SOC_HP_CPU_HAS_MULTIPLE_CORES

    Unfortunately, SystemView does not support tracing from multiple cores. So when tracing from {IDF_TARGET_NAME} with JTAG interfaces in the dual-core mode, two files are generated: one for PRO CPU and another for APP CPU. Users can load each file into separate instances of the tool. For tracing over UART, users can select ``Component config`` > ``Application Level Tracing`` > ``FreeRTOS SystemView Tracing`` in menuconfig Pro or App to choose which CPU has to be traced.

It is uneasy and awkward to analyze data for every core in separate instance of the tool. Fortunately, there is an Eclipse plugin called *Impulse* which can load several trace files, thus making it possible to inspect events from both cores in one view. Also, this plugin has no limitation of 1,000,000 events as compared to the free version of SystemView.

Good instructions on how to install, configure, and visualize data in Impulse from one core can be found `here <https://mcuoneclipse.com/2016/07/31/impulse-segger-systemview-in-eclipse/>`_.

.. note::

    ESP-IDF uses its own mapping for SystemView FreeRTOS events IDs, so users need to replace the original file mapping ``$SYSVIEW_INSTALL_DIR/Description/SYSVIEW_FreeRTOS.txt`` with ``$IDF_PATH/tools/esp_app_trace/SYSVIEW_FreeRTOS.txt``. Also, contents of that ESP-IDF-specific file should be used when configuring SystemView serializer using the above link.

.. only:: SOC_HP_CPU_HAS_MULTIPLE_CORES

    Configure Impulse for Dual Core Traces
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    After installing Impulse and ensuring that it can successfully load trace files for each core in separate tabs, users can add special Multi Adapter port and load both files into one view. To do this, users need to do the following steps in Eclipse:

    1. Open the ``Signal Ports`` view. Go to ``Windows`` > ``Show View`` > ``Other menu``. Find the ``Signal Ports`` view in Impulse folder and double-click it.
    2. In the ``Signal Ports`` view, right-click ``Ports`` and select ``Add`` > ``New Multi Adapter Port``.
    3. In the open dialog box, click ``Add`` and select ``New Pipe/File``.
    4. In the open dialog box, select ``SystemView Serializer`` as Serializer and set path to PRO CPU trace file. Click ``OK``.
    5. Repeat the steps 3-4 for APP CPU trace file.
    6. Double-click the created port. View for this port should open.
    7. Click the ``Start/Stop Streaming`` button. Data should be loaded.
    8. Use the ``Zoom Out``, ``Zoom In`` and ``Zoom Fit`` buttons to inspect data.
    9. For settings measurement cursors and other features, please see `Impulse documentation <https://toem.de/index.php/products/impulse>`_).

    .. note::

        If you have problems with visualization (no data is shown or strange behaviors of zoom action are observed), you can try to delete current signal hierarchy and double-click on the necessary file or port. Eclipse will ask you to create a new signal hierarchy.

.. _app_trace-gcov-source-code-coverage:

Gcov (Source Code Coverage)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

In ESP-IDF projects, code coverage analysis using gcov can be done with the help of `espressif/esp_gcov <https://components.espressif.com/components/espressif/esp_gcov>`_ managed component.
````

## File: docs/en/api-guides/bootloader.rst
````
Bootloader
==========

:link_to_translation:`zh_CN:[中文]`

The ESP-IDF second stage bootloader performs the following functions:

1. Minimal initial configuration of internal modules;
2. Initialize :doc:`/security/flash-encryption` and/or :doc:`Secure Boot </security/secure-boot-v2>` features, if configured;
3. Select the application partition to boot, based on the partition table and ota_data (if any);
4. Load this image to RAM (IRAM & DRAM) and transfer management to the image that was just loaded.

ESP-IDF second stage bootloader is located at the address {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH} in the flash.

For a full description of the startup process including the ESP-IDF second stage bootloader, see :doc:`startup`.

.. _bootloader-compatibility:

Bootloader Compatibility
------------------------

It is recommended to update to newer :doc:`versions of ESP-IDF </versions>`: when they are released. The OTA (over the air) update process can flash new apps in the field but cannot flash a new bootloader. For this reason, the bootloader supports booting apps built from newer versions of ESP-IDF.

The bootloader does not support booting apps from older versions of ESP-IDF. When updating ESP-IDF manually on an existing product that might need to downgrade the app to an older version, keep using the older ESP-IDF bootloader binary as well.

.. note::

    If testing an OTA update for an existing product in production, always test it using the same ESP-IDF bootloader binary that is deployed in production.

.. only:: esp32

    Before ESP-IDF V2.1
    ^^^^^^^^^^^^^^^^^^^

    Bootloaders built from very old versions of ESP-IDF (before ESP-IDF V2.1) perform less hardware configuration than newer versions. When using a bootloader from these early ESP-IDF versions and building a new app, enable the config option :ref:`CONFIG_APP_COMPATIBLE_PRE_V2_1_BOOTLOADERS`.

    Before ESP-IDF V3.1
    ^^^^^^^^^^^^^^^^^^^

    Bootloaders built from versions of ESP-IDF before V3.1 do not support MD5 checksums in the partition table binary. When using a bootloader from these ESP-IDF versions and building a new app, enable the config option :ref:`CONFIG_APP_COMPATIBLE_PRE_V3_1_BOOTLOADERS`.

    Before ESP-IDF V5.1
    ^^^^^^^^^^^^^^^^^^^

    Bootloaders built from versions of ESP-IDF prior to V5.1 do not support :ref:`CONFIG_ESP_SYSTEM_ESP32_SRAM1_REGION_AS_IRAM`. When using a bootloader from these ESP-IDF versions and building a new app, you should not use this option.


SPI Flash Configuration
^^^^^^^^^^^^^^^^^^^^^^^

Each ESP-IDF application or bootloader .bin file contains a header with :ref:`CONFIG_ESPTOOLPY_FLASHMODE`, :ref:`CONFIG_ESPTOOLPY_FLASHFREQ`, :ref:`CONFIG_ESPTOOLPY_FLASHSIZE` embedded in it. These are used to configure the SPI flash during boot.

The :ref:`first-stage-bootloader` reads the :ref:`second-stage-bootloader` header information from flash and uses this information to load the rest of the :ref:`second-stage-bootloader` from flash. However, at this time the system clock speed is lower than configured and not all flash modes are supported. When the :ref:`second-stage-bootloader` then runs, it will reconfigure the flash using values read from the currently selected app binary's header (and NOT from the :ref:`second-stage-bootloader` header). This allows an OTA update to change the SPI flash settings in use.

.. only:: esp32

    Bootloaders prior to ESP-IDF V4.0 used the bootloader's own header to configure the SPI flash, meaning these values could not be changed in an update. To maintain compatibility with older bootloaders, the app re-initializes the flash settings during app startup using the configuration found in the app header.

Log Level
---------

The default bootloader log level is "Info". By setting the :ref:`CONFIG_BOOTLOADER_LOG_LEVEL` option, it is possible to increase or decrease this level. This log level is separate from the log level used in the app (see :doc:`/api-reference/system/log`).

Reducing bootloader log verbosity can improve the overall project boot time by a small amount.

Factory Reset
-------------

Sometimes it is desirable to have a way for the device to fall back to a known-good state, in case of some problem with an update.

To roll back to the original "factory" device configuration and clear any user settings, configure the config item :ref:`CONFIG_BOOTLOADER_FACTORY_RESET` in the bootloader.

The factory reset mechanism allows the device to be factory reset in two ways:

- Clear one or more data partitions. The :ref:`CONFIG_BOOTLOADER_DATA_FACTORY_RESET` option allows users to specify which data partitions will be erased when the factory reset is executed.

  Users can specify the names of partitions as a comma-delimited list with optional spaces for readability. (Like this: ``nvs, phy_init, nvs_custom``).

  Make sure that the names of partitions specified in the option are the same as those found in the partition table. Partitions of type "app" cannot be specified here.

- Boot from "factory" app partition. Enabling the :ref:`CONFIG_BOOTLOADER_OTA_DATA_ERASE` option will cause the device to boot from the default "factory" app partition after a factory reset (or if there is no factory app partition in the partition table then the default ota app partition is selected instead). This reset process involves erasing the OTA data partition which holds the currently selected OTA partition slot. The "factory" app partition slot (if it exists) is never updated via OTA, so resetting to this allows reverting to a "known good" firmware application.

Either or both of these configuration options can be enabled independently.

In addition, the following configuration options control the reset condition:

- :ref:`CONFIG_BOOTLOADER_NUM_PIN_FACTORY_RESET`- The input GPIO number used to trigger a factory reset. This GPIO must be pulled low or high (configurable) on reset to trigger this.

- :ref:`CONFIG_BOOTLOADER_HOLD_TIME_GPIO`- this is hold time of GPIO for reset/test mode (by default 5 seconds). The GPIO must be held continuously for this period of time after reset before a factory reset or test partition boot (as applicable) is performed.

- :ref:`CONFIG_BOOTLOADER_FACTORY_RESET_PIN_LEVEL` - configure whether a factory reset should trigger on a high or low level of the GPIO. If the GPIO has an internal pullup then this is enabled before the pin is sampled, consult the {IDF_TARGET_NAME} datasheet for details on pin internal pullups.

.. only:: SOC_RTC_FAST_MEM_SUPPORTED

    If an application needs to know if the factory reset has occurred, users can call the function :cpp:func:`bootloader_common_get_rtc_retain_mem_factory_reset_state`.

    - If the status is read as true, the function will return the status, indicating that the factory reset has occurred. The function then resets the status to false for subsequent factory reset judgement.
    - If the status is read as false, the function will return the status, indicating that the factory reset has not occurred, or the memory where this status is stored is invalid.

    Note that this feature reserves some RTC FAST memory (the same size as the :ref:`CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP` feature).

.. only:: not SOC_RTC_FAST_MEM_SUPPORTED

    Sometimes an application needs to know if the factory reset has occurred. The {IDF_TARGET_NAME} chip does not have RTC FAST memory, so there is no API to detect it. Instead, there is a workaround: you need an NVS partition that will be erased by the bootloader if factory reset occurs (add this partition to :ref:`CONFIG_BOOTLOADER_DATA_FACTORY_RESET`). In this NVS partition, create a "factory_reset_state" token that will be increased in the application. If the "factory_reset_state" is 0 then the factory reset has occurred.

.. _bootloader_boot_from_test_firmware:

Boot from Test Firmware
------------------------

It is possible to write a special firmware app for testing in production, and boot this firmware when needed. The project partition table will need a dedicated app partition entry for this testing app, type ``app`` and subtype ``test`` (see :doc:`/api-guides/partition-tables`).

Implementing a dedicated test app firmware requires creating a totally separate ESP-IDF project for the test app (each project in ESP-IDF only builds one app). The test app can be developed and tested independently of the main project, and then integrated at production testing time as a pre-compiled .bin file which is flashed to the address of the main project's test app partition.

To support this functionality in the main project's bootloader, set the configuration item :ref:`CONFIG_BOOTLOADER_APP_TEST` and configure the following three items:

- :ref:`CONFIG_BOOTLOADER_NUM_PIN_APP_TEST` - GPIO number to boot test partition. The selected GPIO will be configured as an input with internal pull-up enabled. This GPIO must be pulled low or high (configurable) on reset to trigger this.

  Once the GPIO input is released and the device has been rebooted, the default boot sequence will be enabled again to boot the factory partition or any OTA app partition slot.

- :ref:`CONFIG_BOOTLOADER_HOLD_TIME_GPIO` - this is the hold time of GPIO for reset/test mode (by default 5 seconds). The GPIO must be held continuously for this period of time after reset before a factory reset or test partition boot (as applicable) is performed.

- :ref:`CONFIG_BOOTLOADER_APP_TEST_PIN_LEVEL` - configure whether a test partition boot should trigger on a high or low level of the GPIO. If the GPIO has an internal pull-up, then this is enabled before the pin is sampled. Consult the {IDF_TARGET_NAME} datasheet for details on pin internal pull-ups.

Rollback
--------

Rollback and anti-rollback features must be configured in the bootloader as well.

Consult the :ref:`app_rollback` and :ref:`anti-rollback` sections in the :doc:`OTA API reference document </api-reference/system/ota>`.

.. _bootloader-watchdog:

Watchdog
--------

The chips come equipped with two groups of watchdog timers: Main System Watchdog Timer (MWDT_WDT) and RTC Watchdog Timer (RTC_WDT). Both watchdog timer groups are enabled when the chip is powered up. However, in the bootloader, they will both be disabled. If :ref:`CONFIG_BOOTLOADER_WDT_ENABLE` is set (which is the default behavior), RTC_WDT is re-enabled. It tracks the time from the bootloader is enabled until the user's main function is called. In this scenario, RTC_WDT remains operational and will automatically reset the chip if no application successfully starts within 9 seconds. This functionality is particularly useful in preventing lockups caused by an unstable power source during startup.

- The timeout period can be adjusted by setting :ref:`CONFIG_BOOTLOADER_WDT_TIME_MS` and recompiling the bootloader.
- The RTC Watchdog can be disabled in the bootloader by disabling the :ref:`CONFIG_BOOTLOADER_WDT_ENABLE` setting and recompiling the bootloader. This is not recommended.
- See :ref:`app-hardware-watchdog-timers` to learn how RTC_WDT is used in the application.

.. _bootloader-size:

Bootloader Size
---------------

{IDF_TARGET_MAX_BOOTLOADER_SIZE:default = "80 KB (0x14000 bytes)", esp32 = "48 KB (0xC000 bytes)", esp32s2, esp32s3, esp32c2, esp32c3, esp32c6, esp32h2, esp32h21, esp32p4 = "64 KB (0x10000 bytes)"}
{IDF_TARGET_MAX_PARTITION_TABLE_OFFSET:default = "0x11000", esp32 = "0xE000", esp32c5, esp32h4 = "0x17000", esp32c61 = "0x15000", esp32p4 = "0x13000"}
.. Above is calculated as:
    0x1000 at start of flash + IDF_TARGET_MAX_BOOTLOADER_SIZE + 0x1000 signature sector // for esp32
    0x0 at start of flash + IDF_TARGET_MAX_BOOTLOADER_SIZE + 0x1000 signature sector // for esp32s2, esp32s3, esp32c2, esp32c3, esp32c6, esp32c61, esp32h2, esp32h21
    0x2000 at start of flash + IDF_TARGET_MAX_BOOTLOADER_SIZE + 0x1000 signature sector // for Key Manager supported targets: esp32c5, esp32h4, esp32p4

When enabling additional bootloader functions, including :doc:`/security/flash-encryption` or Secure Boot, and especially if setting a high :ref:`CONFIG_BOOTLOADER_LOG_LEVEL` level, then it is important to monitor the bootloader .bin file's size.

When using the default :ref:`CONFIG_PARTITION_TABLE_OFFSET` value 0x8000, the size limit is {IDF_TARGET_CONFIG_PARTITION_TABLE_OFFSET} bytes.

If the bootloader binary is too large, then the bootloader build will fail with an error "Bootloader binary size [..] is too large for partition table offset". If the bootloader binary is flashed anyhow then the {IDF_TARGET_NAME} will fail to boot - errors will be logged about either invalid partition table or invalid bootloader checksum.

Options to work around this are:

- Set :ref:`bootloader compiler optimization <CONFIG_BOOTLOADER_COMPILER_OPTIMIZATION>` back to "Size" if it has been changed from this default value.
- Reduce :ref:`bootloader log level <CONFIG_BOOTLOADER_LOG_LEVEL>`. Setting log level to Warning, Error or None all significantly reduce the final binary size (but may make it harder to debug).
- Set :ref:`CONFIG_PARTITION_TABLE_OFFSET` to a higher value than 0x8000, to place the partition table later in the flash. This increases the space available for the bootloader. If the :doc:`partition table </api-guides/partition-tables>` CSV file contains explicit partition offsets, they will need changing so no partition has an offset lower than ``CONFIG_PARTITION_TABLE_OFFSET + 0x1000``. (This includes the default partition CSV files supplied with ESP-IDF.)

When Secure Boot V2 is enabled, there is also an absolute binary size limit of {IDF_TARGET_MAX_BOOTLOADER_SIZE} (excluding the 4 KB signature), because the bootloader is first loaded into a fixed size buffer for verification.

Fast Boot from Deep-Sleep
-------------------------

The bootloader has the :ref:`CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP` option which allows the wake-up time from Deep-sleep to be reduced (useful for reducing power consumption). This option is available when the :ref:`CONFIG_SECURE_BOOT` option is disabled or :ref:`CONFIG_SECURE_BOOT_INSECURE` is enabled along with Secure Boot. The reduction in time is achieved by ignoring image verification.

.. only:: SOC_RTC_FAST_MEM_SUPPORTED

    During the first boot, the bootloader stores the address of the application being launched in the RTC FAST memory. After waking up from deep sleep, this address is used to boot the application again without any checks, resulting in a significantly faster load.

.. only:: not SOC_RTC_FAST_MEM_SUPPORTED

    The {IDF_TARGET_NAME} does not have RTC memory, so a running partition cannot be saved there; instead, the entire partition table is read to select the correct application. During wake-up, the selected application is loaded without any checks, resulting in a significantly faster load.

Custom Bootloader
-----------------

The current bootloader implementation allows a project to extend it or modify it. There are two ways of doing it: by implementing hooks or by overriding it. Both ways are presented in :example:`custom_bootloader` folder in ESP-IDF examples:

* :example:`custom_bootloader/bootloader_hooks` presents how to connect some hooks to the bootloader initialization
* :example:`custom_bootloader/bootloader_override` presents how to override the bootloader implementation

In the bootloader space, you cannot use the drivers and functions from other components unless they explicitly support run in bootloader. If necessary, then the required functionality should be placed in the project's `bootloader_components` directory (note that this will increase its size). Examples of components that can be used in the bootloader are:

* :example:`storage/nvs/nvs_bootloader`

If the bootloader grows too large then it can collide with the partition table, which is flashed at offset 0x8000 by default. Increase the :ref:`partition table offset <CONFIG_PARTITION_TABLE_OFFSET>` value to place the partition table later in the flash. This increases the space available for the bootloader.

.. only:: SOC_RECOVERY_BOOTLOADER_SUPPORTED

    Recovery Bootloader
    -------------------

    The {IDF_TARGET_NAME} introduces Recovery Bootloader and Anti-rollback Bootloader features, implemented in the ROM bootloader to enhance device security and reliability during OTA updates.

    The recovery bootloader feature enables safe OTA updates of the bootloader itself. When the eFuse field ``ESP_EFUSE_RECOVERY_BOOTLOADER_FLASH_SECTOR`` is set, it specifies the flash address (in sectors) of the recovery bootloader. If the primary bootloader at {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH} fails to load, the ROM bootloader attempts to load the recovery bootloader from this address.

    - The eFuse can be set using ``espefuse.py`` or by calling :cpp:func:`esp_efuse_set_recovery_bootloader_offset()` in the user application.
    - The address can be set using ``CONFIG_BOOTLOADER_RECOVERY_OFFSET``. This value must be a multiple of the flash sector size (0x1000 bytes). The Kconfig option helps ensure that the recovery bootloader does not overlap with existing partitions.
    - Note that the eFuse field stores the offset in sectors. Setting it to the maximum value ``0xFFF`` disables the feature.
    - The recovery bootloader image at the ``CONFIG_BOOTLOADER_RECOVERY_OFFSET`` is not flashed by default. It can be written as part of the OTA update process.

    The example below shows the bootloader log when the primary bootloader fails to load and the recovery bootloader is loaded instead.

    .. code-block:: none

        ESP-ROM:esp32c5-eco2-20250121
        Build:Jan 21 2025
        rst:0x1 (POWERON),boot:0x18 (SPI_FAST_FLASH_BOOT)
        invalid header: 0xffffffff
        invalid header: 0xffffffff
        invalid header: 0xffffffff
        PRIMARY - FAIL
        Loading RECOVERY Bootloader...
        SPI mode:DIO, clock div:1
        load:0x408556b0,len:0x17cc
        load:0x4084bba0,len:0xdac
        load:0x4084e5a0,len:0x3140
        entry 0x4084bbaa

        I (46) boot: ESP-IDF v6.0-dev-172-g12c5d730097-dirty 2nd stage bootloader
        I (46) boot: compile time May 22 2025 12:41:59
        I (47) boot: chip revision: v1.0
        I (48) boot: efuse block revision: v0.1
        I (52) boot.esp32c5: SPI Speed      : 80MHz
        I (55) boot.esp32c5: SPI Mode       : DIO
        I (59) boot.esp32c5: SPI Flash Size : 4MB
        I (63) boot: Enabling RNG early entropy source...
        I (67) boot: Partition Table:
        ...

    Anti-Rollback Feature
    ^^^^^^^^^^^^^^^^^^^^^

    The anti-rollback feature prevents downgrading to an older, potentially vulnerable bootloader version. The bootloader header includes a security version, defined by ``CONFIG_BOOTLOADER_SECURE_VERSION``. When ``EFUSE_BOOTLOADER_ANTI_ROLLBACK_EN`` is set, the ROM bootloader checks this version against the value stored in ``EFUSE_BOOTLOADER_ANTI_ROLLBACK_SECURE_VERSION``. Only bootloaders with a version greater than or equal to the eFuse value are allowed to boot.

    - The ROM bootloader can update the secure version in eFuse if ``EFUSE_BOOTLOADER_ANTI_ROLLBACK_SECURE_VERSION_UPDATE_IN_ROM`` is set.
    - The secure version value is incremented as new bootloader versions are deployed, and cannot be decreased.
    - If the secure version in eFuse is not updated in the ROM bootloader, then the application can update it using the :cpp:func:`esp_efuse_write_field_blob` function.

    Relevant eFuses
    ^^^^^^^^^^^^^^^

    - ``EFUSE_RECOVERY_BOOTLOADER_FLASH_SECTOR`` (12 bits): Flash sector address for the recovery bootloader. Default value is 0 (disabled), set any other value to enable, 0xFFF to permanently disable.
    - ``EFUSE_BOOTLOADER_ANTI_ROLLBACK_EN`` (1 bit): Enables anti-rollback check in the ROM bootloader.
    - ``EFUSE_BOOTLOADER_ANTI_ROLLBACK_SECURE_VERSION`` (4 bits): Secure version for anti-rollback protection. The value increases as bits are set—0x0, 0x1, 0x3, 0x7, 0xF.
    - ``EFUSE_BOOTLOADER_ANTI_ROLLBACK_SECURE_VERSION_UPDATE_IN_ROM`` (1 bit): Allows the ROM bootloader to update the secure version in eFuse.

    .. note::

        Use these features to improve device security and reliability during OTA updates. Carefully plan eFuse programming, as these settings are permanent and may affect future update strategies.
````

## File: docs/en/api-guides/build-system.rst
````
Build System
************

:link_to_translation:`zh_CN:[中文]`

This document explains the implementation of the ESP-IDF build system and the concept of "components". Read this document if you want to know how to organize and build a new ESP-IDF project or component.


Overview
========

An ESP-IDF project can be seen as an amalgamation of a number of components. For example, for a web server that shows the current humidity, there could be:

- The ESP-IDF base libraries (libc, ROM bindings, etc)
- The Wi-Fi drivers
- A TCP/IP stack
- The FreeRTOS operating system
- A web server
- A driver for the humidity sensor
- Main code tying it all together

ESP-IDF makes these components explicit and configurable. To do that, when a project is compiled, the build system will look up all the components in the ESP-IDF directories, the project directories and (optionally) in additional custom component directories. It then allows the user to configure the ESP-IDF project using a text-based menu system to customize each component. After the components in the project are configured, the build system will compile the project.


Concepts
--------

- A ``project`` is a directory that contains all the files and configuration to build a single ``app`` (executable), as well as additional supporting elements such as a partition table, data partitions or filesystem partitions, and a bootloader.

- ``Project configuration`` is held in a single file called ``sdkconfig`` in the root directory of the project. This configuration file is modified via ``idf.py menuconfig`` to customize the configuration of the project. A single project contains exactly one project configuration.

- An ``app`` is an executable that is built by ESP-IDF. A single project will usually build two apps - a "project app" (the main executable, ie your custom firmware) and a "bootloader app" (the initial bootloader program which launches the project app).

- ``Components`` are modular pieces of standalone code that are compiled into static libraries (.a files) and linked to an app. Some are provided by ESP-IDF itself, others may be sourced from other places.

- ``Target`` is the hardware for which an application is built. A full list of supported targets in your version of ESP-IDF can be seen by running `idf.py --list-targets`.

Some things are not part of the project:

- ``ESP-IDF`` is not part of the project. Instead, it is standalone, and linked to the project via the ``IDF_PATH`` environment variable which holds the path of the ``esp-idf`` directory. This allows the ESP-IDF framework to be decoupled from your project.

- The toolchain for compilation is not part of the project. The toolchain should be installed in the system command line PATH.


Using the Build System
======================

.. _idf.py:

idf.py
------

The ``idf.py`` command-line tool provides a front-end for easily managing your project builds. It manages the following tools:

- CMake_, which configures the project to be built
- Ninja_ which builds the project
- `esptool.py`_ for flashing the target.

For more details about configuring the build system using ``idf.py``, please refer to :doc:`IDF Frontend <tools/idf-py>`.


Using CMake Directly
--------------------

:ref:`idf.py` is a wrapper around CMake_ for convenience. However, you can also invoke CMake directly.

.. highlight:: bash

When ``idf.py`` does something, it prints each command that it runs for easy reference. For example, the ``idf.py build`` command is the same as running these commands in a bash shell (or similar commands for Windows Command Prompt)::

  mkdir -p build
  cd build
  cmake .. -G Ninja   # or 'Unix Makefiles'
  ninja

In the above list, the ``cmake`` command configures the project and generates build files for use with the final build tool. In this case, the final build tool is Ninja_: running ``ninja`` actually builds the project.

It's not necessary to run ``cmake`` more than once. After the first build, you only need to run ``ninja`` each time. ``ninja`` will automatically re-invoke ``cmake`` if the project needs reconfiguration.

If using CMake with ``ninja`` or ``make``, there are also targets for more of the ``idf.py`` sub-commands. For example, running ``make menuconfig`` or ``ninja menuconfig`` in the build directory will work the same as ``idf.py menuconfig``.

.. note::

   If you're already familiar with CMake_, you may find the ESP-IDF CMake-based build system unusual because it wraps a lot of CMake's functionality to reduce boilerplate. See `writing pure CMake components`_ for some information about writing more "CMake style" components.


.. _flash-with-ninja-or-make:

Flashing with Ninja or Make
^^^^^^^^^^^^^^^^^^^^^^^^^^^

It's possible to build and flash directly from ninja or make by running a target like::

  ninja flash

Or::

  make app-flash

Available targets are: ``flash``, ``app-flash`` (app only), ``bootloader-flash`` (bootloader only).

When flashing this way, optionally set the ``ESPPORT`` and ``ESPBAUD`` environment variables to specify the serial port and baud rate. You can set environment variables in your operating system or IDE project. Alternatively, set them directly on the command line::

  ESPPORT=/dev/ttyUSB0 ninja flash

.. note::

  Providing environment variables at the start of the command like this is Bash shell Syntax. It will work on Linux and macOS. It won't work when using Windows Command Prompt, but it will work when using Bash-like shells on Windows.

Or::

  make -j3 app-flash ESPPORT=COM4 ESPBAUD=2000000

.. note::

  Providing variables at the end of the command line is ``make`` syntax, and works for ``make`` on all platforms.


Using CMake in an IDE
---------------------

You can also use an IDE with CMake integration. The IDE will want to know the path to the project's ``CMakeLists.txt`` file. IDEs with CMake integration often provide their own build tools (CMake calls these "generators") to build the source files as part of the IDE.

When adding custom non-build steps like "flash" to the IDE, it is recommended to execute ``idf.py`` for these "special" commands.

For more detailed information about integrating ESP-IDF with CMake into an IDE, see `Build System Metadata`_.

.. _setting-python-interpreter:

Setting up the Python Interpreter
---------------------------------

ESP-IDF works well with Python version 3.10+.

``idf.py`` and other Python scripts will run with the default Python interpreter, i.e., ``python``. You can switch to a different one like ``python3 $IDF_PATH/tools/idf.py ...``, or you can set up a shell alias or another script to simplify the command.

If using CMake directly, running ``cmake -D PYTHON=python3 ...`` will cause CMake to override the default Python interpreter.

If using an IDE with CMake, setting the ``PYTHON`` value as a CMake cache override in the IDE UI will override the default Python interpreter.

To manage the Python version more generally via the command line, check out the tools pyenv_ or virtualenv_. These let you change the default Python version.


.. _example-project-structure:

Example Project
===============

.. highlight:: none

An example project directory tree might look like this:

.. code-block:: none

    - myProject/
                 - CMakeLists.txt
                 - sdkconfig
                 - dependencies.lock
                 - bootloader_components/ - boot_component/ - CMakeLists.txt
                                                            - Kconfig
                                                            - src1.c
                 - components/ - component1/ - CMakeLists.txt
                                             - Kconfig
                                             - src1.c
                               - component2/ - CMakeLists.txt
                                             - Kconfig
                                             - src1.c
                                             - include/ - component2.h
                 - managed_components/ - namespace__component-name/ - CMakelists.txt
                                                                    - src1.c
                                                                    - idf_component.yml
                                                                    - include/ - src1.h
                 - main/       - CMakeLists.txt
                               - src1.c
                               - src2.c
                               - idf_component.yml
                 - build/

This example "myProject" contains the following elements:

- A top-level project CMakeLists.txt file. This is the primary file which CMake uses to learn how to build the project; and may set project-wide CMake variables. It includes the file :idf_file:`/tools/cmake/project.cmake` which implements the rest of the build system. Finally, it sets the project name and defines the project.

- "sdkconfig" project configuration file. This file is created/updated when ``idf.py menuconfig`` runs, and holds the configuration for all of the components in the project (including ESP-IDF itself). The ``sdkconfig`` file may or may not be added to the source control system of the project. More information about this file can be found in the :ref:`sdkconfig file <sdkconfig-file>` section in the Configuration Guide.

- "dependencies.lock" file contains the list of all managed components, and their versions, that are currently in used in the project. The ``dependencies.lock`` file is generated or updated automatically when IDF Component Manager is used to add or update project components. So this file should never be edited manually! If the project does not have ``idf_component.yml`` files in any of its components, ``dependencies.lock`` will not be created.

- Optional "idf_component.yml" file contains metadata about the component and its dependencies. It is used by the IDF Component Manager to download and resolve these dependencies. More information about this file can be found in the `idf_component.yml <https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html>`_ section.

- Optional "bootloader_components" directory contains components that need to be compiled and linked inside the bootloader project. A project does not have to contain custom bootloader components of this kind, but it can be useful in case the bootloader needs to be modified to embed new features.

- Optional "components" directory contains components that are part of the project. A project does not have to contain custom components of this kind, but it can be useful for structuring reusable code or including third-party components that aren't part of ESP-IDF. Alternatively, ``EXTRA_COMPONENT_DIRS`` can be set in the top-level CMakeLists.txt to look for components in other places.

- "main" directory is a special component that contains source code for the project itself. "main" is a default name, the CMake variable ``COMPONENT_DIRS`` includes this component but you can modify this variable. See the :ref:`renaming main <rename-main>` section for more info. If you have a lot of source files in your project, we recommend grouping most into components instead of putting them all in "main".

- "build" directory is where the build output is created. This directory is created by ``idf.py`` if it doesn't already exist. CMake configures the project and generates interim build files in this directory. Then, after the main build process is run, this directory will also contain interim object files and libraries as well as final binary output files. This directory is usually not added to source control or distributed with the project source code.

- "managed_components" directory is created by the IDF Component Manager to store components managed by this tool. Each managed component typically includes a ``idf_component.yml`` manifest file defining the component's metadata, such as version and dependencies. However, for components sourced from Git repositories, the manifest file is optional. Users should avoid manually modifying the contents of the "managed_components" directory. If alterations are needed, the component can be copied to the ``components`` directory. The "managed_components" directory is usually not versioned in Git and not distributed with the project source code.

Component directories each contain a component ``CMakeLists.txt`` file. This file contains variable definitions to control the build process of the component, and its integration into the overall project. See `Component CMakeLists Files`_ for more details.

Each component may also include a ``Kconfig`` file defining the `component configuration`_ options that can be set via ``menuconfig``. Some components may also include ``Kconfig.projbuild`` and ``project_include.cmake`` files, which are special files for `overriding parts of the project`_.


Project CMakeLists File
=======================

Each project has a single top-level ``CMakeLists.txt`` file that contains build settings for the entire project. By default, the project CMakeLists can be quite minimal.


Minimal Example CMakeLists
--------------------------

.. highlight:: cmake

Minimal project::

      cmake_minimum_required(VERSION 3.22)
      include($ENV{IDF_PATH}/tools/cmake/project.cmake)
      project(myProject)


.. _project-mandatory-parts:

Mandatory Parts
---------------

The inclusion of these three lines, in the order shown above, is necessary for every project:

- ``cmake_minimum_required(VERSION 3.22)`` tells CMake the minimum version that is required to build the project. ESP-IDF is designed to work with CMake 3.22 or newer. This line must be the first line in the CMakeLists.txt file.
- ``include($ENV{IDF_PATH}/tools/cmake/project.cmake)`` pulls in the rest of the CMake functionality to configure the project, discover all the components, etc.
- ``project(myProject)`` creates the project itself, and specifies the project name. The project name is used for the final binary output files of the app - ie ``myProject.elf``, ``myProject.bin``. Only one project can be defined per CMakeLists file.


.. _optional_project_variable:

Optional Project Variables
--------------------------

These variables all have default values that can be overridden for custom behavior. Look in :idf_file:`/tools/cmake/project.cmake` for all of the implementation details.

- ``COMPONENT_DIRS``: Directories to search for components. Defaults to ``IDF_PATH/components``, ``PROJECT_DIR/components``, and ``EXTRA_COMPONENT_DIRS``. Override this variable if you don't want to search for components in these places.

- ``EXTRA_COMPONENT_DIRS``: Optional list of additional directories to search for components. Paths can be relative to the project directory, or absolute.

- ``COMPONENTS``: A list of component names to build into the project. Defaults to all components found in the ``COMPONENT_DIRS`` directories. Use this variable to "trim down" the project for faster build times. Note that any component which "requires" another component via the REQUIRES or PRIV_REQUIRES arguments on component registration will automatically have it added to this list, so the ``COMPONENTS`` list can be very short. The ``MINIMAL_BUILD`` :ref:`build property <cmake-build-properties>` can be used as an alternative to specifying only the ``main`` component in ``COMPONENTS``.

- ``BOOTLOADER_IGNORE_EXTRA_COMPONENT``: Optional list of components, placed in ``bootloader_components/``, that should be ignored by the bootloader compilation. Use this variable if a bootloader component needs to be included conditionally inside the project.

- ``BOOTLOADER_EXTRA_COMPONENT_DIRS``: Optional list of additional directories to search for components to be compiled as part of the bootloader. Please note that this is a build property.

Any paths in these variables can be absolute paths, or set relative to the project directory.

To set these variables, use the `cmake set command <cmake set_>`_ ie ``set(VARIABLE "VALUE")``. The ``set()`` commands should be placed after the ``cmake_minimum(...)`` line but before the ``include(...)`` line.


.. _rename-main:

Renaming ``main`` Component
----------------------------

The build system provides special treatment to the ``main`` component. It is a component that gets automatically added to the build provided that it is in the expected location, PROJECT_DIR/main. All other components in the build are also added as its dependencies, saving the user from hunting down dependencies and providing a build that works right out of the box. Renaming the ``main`` component causes the loss of these behind-the-scenes heavy lifting, requiring the user to specify the location of the newly renamed component and manually specify its dependencies. Specifically, the steps to renaming ``main`` are as follows:

1. Rename ``main`` directory.
2. Set ``EXTRA_COMPONENT_DIRS`` in the project CMakeLists.txt to include the renamed ``main`` directory.
3. Specify the dependencies in the renamed component's CMakeLists.txt file via REQUIRES or PRIV_REQUIRES arguments :ref:`on component registration <cmake_minimal_component_cmakelists>`.


Overriding Default Build Specifications
---------------------------------------

The build sets some global build specifications (compile flags, definitions, etc.) that gets used in compiling all sources from all components.

.. highlight:: cmake

For example, one of the default build specifications set is the compile option ``-Wextra``. Suppose a user wants to use override this with ``-Wno-extra``,
it should be done after ``project()``::


    cmake_minimum_required(VERSION 3.22)
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(myProject)

    idf_build_set_property(COMPILE_OPTIONS "-Wno-error" APPEND)

This ensures that the compile options set by the user won't be overridden by the default build specifications, since the latter are set inside ``project()``.


.. _component-directories:

Component CMakeLists Files
==========================

Each project contains one or more components. Components can be part of ESP-IDF, part of the project's own components directory, or added from custom component directories (:ref:`see above <component-directories>`).

A component is any directory in the ``COMPONENT_DIRS`` list which contains a ``CMakeLists.txt`` file.


Searching for Components
------------------------

The list of directories in ``COMPONENT_DIRS`` is searched for the project's components. Directories in this list can either be components themselves (ie they contain a `CMakeLists.txt` file), or they can be top-level directories whose sub-directories are components.

When CMake runs to configure the project, it logs the components included in the build. This list can be useful for debugging the inclusion/exclusion of certain components.


.. _cmake-components-same-name:

Multiple Components with the Same Name
--------------------------------------

When ESP-IDF is collecting all the components to compile, the search precedence is as follows (from highest to lowest):

* Project components
* Components from ``EXTRA_COMPONENT_DIRS``
* Project managed components, downloaded by the IDF Component Manager into ``PROJECT_DIR/managed_components``, unless the IDF Component Manager is disabled.
* ESP-IDF components (``IDF_PATH/components``)

If two or more of these directories contain component sub-directories with the same name, the component with higher precedence is used. This allows, for example, overriding ESP-IDF components with a modified version by copying that component from the ESP-IDF components directory to the project components directory and then modifying it there. If used in this way, the ESP-IDF directory itself can remain untouched.

.. note::

  If a component is overridden in an existing project by moving it to a new location, the project will not automatically see the new component path. Run ``idf.py reconfigure`` (or delete the project build folder) and then build again.


.. _cmake_minimal_component_cmakelists:

Minimal Component CMakeLists
----------------------------

.. highlight:: cmake

The minimal component ``CMakeLists.txt`` file simply registers the component to the build system using ``idf_component_register``::

  idf_component_register(SRCS "foo.c" "bar.c"
                         INCLUDE_DIRS "include"
                         REQUIRES mbedtls)

- ``SRCS`` is a list of source files (``*.c``, ``*.cpp``, ``*.cc``, ``*.S``). These source files will be compiled into the component library.
- ``INCLUDE_DIRS`` is a list of directories to add to the global include search path for any component which requires this component, and also the main source files.
- ``REQUIRES`` is not actually required, but it is very often required to declare what other components this component will use. See :ref:`component requirements <component-requirements>`.

A library with the name of the component will be built and linked to the final app.

Directories are usually specified relative to the ``CMakeLists.txt`` file itself, although they can be absolute.

There are other arguments that can be passed to ``idf_component_register``. These arguments are discussed :ref:`here <cmake-component-register>`.

See `example component requirements`_ and  `example component CMakeLists`_ for more complete component ``CMakeLists.txt`` examples.


.. _preset_component_variables:

Preset Component Variables
--------------------------

The following component-specific variables are available for use inside component CMakeLists, but should not be modified:

- ``COMPONENT_DIR``: The component directory. Evaluates to the absolute path of the directory containing ``CMakeLists.txt``. The component path cannot contain spaces. This is the same as the ``CMAKE_CURRENT_SOURCE_DIR`` variable.
- ``COMPONENT_NAME``: Name of the component. Same as the name of the component directory.
- ``COMPONENT_ALIAS``: Alias of the library created internally by the build system for the component.
- ``COMPONENT_LIB``: Name of the library created internally by the build system for the component.
- ``COMPONENT_VERSION``: Component version specified by idf_component.yml and set by IDF Component Manager.

The following variables are set at the project level, but available for use in component CMakeLists:

- ``CONFIG_*``: Each value in the project configuration has a corresponding variable available in cmake. All names begin with ``CONFIG_``. More information on how the project configuration works, please visit :ref:`Project Configuration Guide <project-configuration-guide>`.
- ``ESP_PLATFORM``: Set to 1 when the CMake file is processed within the ESP-IDF build system.


Build/Project Variables
-----------------------

The following are some project/build variables that are available as build properties and whose values can be queried using ``idf_build_get_property`` from the component CMakeLists.txt:

- ``PROJECT_NAME``: Name of the project, as set in project CMakeLists.txt file.
- ``PROJECT_DIR``: Absolute path of the project directory containing the project CMakeLists. Same as the ``CMAKE_SOURCE_DIR`` variable.
- ``COMPONENTS``: Names of all components that are included in this build, formatted as a semicolon-delimited CMake list.
- ``IDF_VER``: Git version of ESP-IDF (produced by ``git describe``)
- ``IDF_VERSION_MAJOR``, ``IDF_VERSION_MINOR``, ``IDF_VERSION_PATCH``: Components of ESP-IDF version, to be used in conditional expressions. Note that this information is less precise than that provided by ``IDF_VER`` variable. ``v4.0-dev-*``, ``v4.0-beta1``, ``v4.0-rc1`` and ``v4.0`` will all have the same values of ``IDF_VERSION_*`` variables, but different ``IDF_VER`` values.
- ``IDF_TARGET``: Name of the target for which the project is being built.
- ``PROJECT_VER``: Project version.

  * If :ref:`CONFIG_APP_PROJECT_VER_FROM_CONFIG` option is set, the value of :ref:`CONFIG_APP_PROJECT_VER` will be used.
  * Else, if ``PROJECT_VER`` variable is set in project CMakeLists.txt file, its value will be used.
  * Else, if the ``PROJECT_DIR/version.txt`` exists, its contents will be used as ``PROJECT_VER``.
  * Else, if ``VERSION`` argument is passed to the ``project()`` call in the CMakeLists.txt file as ``project(... VERSION x.y.z.w )`` then it will be used as ``PROJECT_VER``. The ``VERSION`` argument must be compliant with the `cmake standard <https://cmake.org/cmake/help/v3.22/command/project.html>`_.
  * Else, if the project is located inside a Git repository, the output of git description will be used.
  * Otherwise, ``PROJECT_VER`` will be "1".
- ``EXTRA_PARTITION_SUBTYPES``: CMake list of extra partition subtypes. Each subtype description is a comma-separated string with ``type_name, subtype_name, numeric_value`` format. Components may add new subtypes by appending them to this list.

Other build properties are listed :ref:`here <cmake-build-properties>`.


.. _component_build_control:

Controlling Component Compilation
---------------------------------

.. highlight:: cmake

To pass compiler options when compiling source files belonging to a particular component, use the `target_compile_options`_ function::

  target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable)

To apply the compilation flags to a single source file, use the CMake `set_source_files_properties`_ command::

    set_source_files_properties(mysrc.c
        PROPERTIES COMPILE_FLAGS
        -Wno-unused-variable
    )

This can be useful if there is upstream code that emits warnings.

.. note::

    CMake `set_source_files_properties`_ command is not applicable when the source files have been populated with help of the ``SRC_DIRS`` variable in ``idf_component_register``. See :ref:`cmake-file-globbing` for more details.

When using these commands, place them after the call to ``idf_component_register`` in the component CMakeLists file.


.. _component-configuration:

Component Configuration
=======================

Each component can also have a ``Kconfig`` file, alongside ``CMakeLists.txt``. This contains configuration settings to add to the configuration menu for this component.

These settings are found under the "Component Settings" menu when menuconfig is run.

To create a component Kconfig file, it is easiest to start with one of the Kconfig files distributed with ESP-IDF.

For an example, see `Adding conditional configuration`_. For a more detailed guide, see :ref:`Component Configuration Guide <component-configuration-guide>`.


Preprocessor Definitions
========================

The ESP-IDF build system adds the following C preprocessor definitions on the command line:

- ``ESP_PLATFORM`` : Can be used to detect that build happens within ESP-IDF.
- ``IDF_VER`` : Defined to a git version string.  E.g. ``v2.0`` for a tagged release or ``v1.0-275-g0efaa4f`` for an arbitrary commit.


.. _component-requirements:

Component Requirements
======================

When compiling each component, the ESP-IDF build system recursively evaluates its dependencies. This means each component needs to declare the components that it depends on ("requires").


When Writing a Component
------------------------

.. code-block:: cmake

   idf_component_register(...
                          REQUIRES mbedtls
                          PRIV_REQUIRES console spiffs)

- ``REQUIRES`` should be set to all components whose header files are #included from the *public* header files of this component.

- ``PRIV_REQUIRES`` should be set to all components whose header files are #included from *any source files* in this component, unless already listed in ``REQUIRES``. Also, any component which is required to be linked in order for this component to function correctly.

- The values of ``REQUIRES`` and ``PRIV_REQUIRES`` should not depend on any configuration options (``CONFIG_xxx`` macros). This is because requirements are expanded before the configuration is loaded. Other component variables (like include paths or source files) can depend on configuration options.

- Not setting either or both ``REQUIRES`` variables is fine. If the component has no requirements except for the `Common component requirements`_ needed for RTOS, libc, etc.

If a component only supports some target chips (values of ``IDF_TARGET``) then it can specify ``REQUIRED_IDF_TARGETS`` in the ``idf_component_register`` call to express these requirements. In this case, the build system will generate an error if the component is included in the build, but does not support the selected target.

.. note::

  In CMake terms, ``REQUIRES`` & ``PRIV_REQUIRES`` are approximate wrappers around the CMake functions ``target_link_libraries(... PUBLIC ...)`` and ``target_link_libraries(... PRIVATE ...)``.


.. _example component requirements:

Example of Component Requirements
---------------------------------

Imagine there is a ``car`` component, which uses the ``engine`` component, which uses the ``spark_plug`` component:

.. code-block:: none

    - autoProject/
                 - CMakeLists.txt
                 - components/ - car/ - CMakeLists.txt
                                         - car.c
                                         - car.h
                               - engine/ - CMakeLists.txt
                                         - engine.c
                                         - include/ - engine.h
                               - spark_plug/  - CMakeLists.txt
                                              - spark_plug.c
                                              - spark_plug.h


Car Component
^^^^^^^^^^^^^

.. highlight:: c

The ``car.h`` header file is the public interface for the ``car`` component. This header includes ``engine.h`` directly because it uses some declarations from this header::

  /* car.h */
  #include "engine.h"

  #ifdef ENGINE_IS_HYBRID
  #define CAR_MODEL "Hybrid"
  #endif

And car.c includes ``car.h`` as well::

  /* car.c */
  #include "car.h"

This means the ``car/CMakeLists.txt`` file needs to declare that ``car`` requires ``engine``:

.. code-block:: cmake

  idf_component_register(SRCS "car.c"
                    INCLUDE_DIRS "."
                    REQUIRES engine)

- ``SRCS`` gives the list of source files in the ``car`` component.
- ``INCLUDE_DIRS`` gives the list of public include directories for this component. Because the public interface is ``car.h``, the directory containing ``car.h`` is listed here.
- ``REQUIRES`` gives the list of components required by the public interface of this component. Because ``car.h`` is a public header and includes a header from ``engine``, we include ``engine`` here. This makes sure that any other component which includes ``car.h`` will be able to recursively include the required ``engine.h`` also.


Engine Component
^^^^^^^^^^^^^^^^

.. highlight:: c

The ``engine`` component also has a public header file ``include/engine.h``, but this header is simpler::

  /* engine.h */
  #define ENGINE_IS_HYBRID

  void engine_start(void);

The implementation is in ``engine.c``::

  /* engine.c */
  #include "engine.h"
  #include "spark_plug.h"

  ...

In this component, ``engine`` depends on ``spark_plug`` but this is a private dependency. ``spark_plug.h`` is needed to compile ``engine.c``, but not needed to include ``engine.h``.

This means that the ``engine/CMakeLists.txt`` file can use ``PRIV_REQUIRES``:

.. code-block:: cmake

  idf_component_register(SRCS "engine.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES spark_plug)

As a result, source files in the ``car`` component don't need the ``spark_plug`` include directories added to their compiler search path. This can speed up compilation, and stops compiler command lines from becoming longer than necessary.


Spark Plug Component
^^^^^^^^^^^^^^^^^^^^

The ``spark_plug`` component doesn't depend on anything else. It has a public header file ``spark_plug.h``, but this doesn't include headers from any other components.

This means that the ``spark_plug/CMakeLists.txt`` file doesn't need any ``REQUIRES`` or ``PRIV_REQUIRES`` clauses:

.. code-block:: cmake

  idf_component_register(SRCS "spark_plug.c"
                    INCLUDE_DIRS ".")


Source File Include Directories
-------------------------------

Each component's source file is compiled with these include path directories, as specified in the passed arguments to ``idf_component_register``:

.. code-block:: cmake

  idf_component_register(..
                         INCLUDE_DIRS "include"
                         PRIV_INCLUDE_DIRS "other")


- The current component's ``INCLUDE_DIRS`` and ``PRIV_INCLUDE_DIRS``.
- The ``INCLUDE_DIRS`` belonging to all other components listed in the ``REQUIRES`` and ``PRIV_REQUIRES`` parameters (ie all the current component's public and private dependencies).
- Recursively, all of the ``INCLUDE_DIRS`` of those components ``REQUIRES`` lists (ie all public dependencies of this component's dependencies, recursively expanded).


Main Component Requirements
---------------------------

The component named ``main`` is special because it automatically requires all other components in the build. So it's not necessary to pass ``REQUIRES`` or ``PRIV_REQUIRES`` to this component. See :ref:`renaming main <rename-main>` for a description of what needs to be changed if no longer using the ``main`` component.


.. _component-common-requirements:

Common Component Requirements
-----------------------------

To avoid duplication, every component automatically requires some "common" IDF components even if they are not mentioned explicitly. Headers from these components can always be included.

The list of common components is: cxx, esp_libc, freertos, esp_hw_support, heap, log, soc, hal, esp_rom, esp_common, esp_system, xtensa/riscv.


Including Components in the Build
---------------------------------

- By default, every component is included in the build.
- If you set the ``COMPONENTS`` variable to a minimal list of components used directly by your project, then the build will expand to also include required components. The full list of components will be:

  - Components mentioned explicitly in ``COMPONENTS``.
  - Those components' requirements (evaluated recursively).
  - The :ref:`common components <component-common-requirements>` that every component depends on.

- Setting ``COMPONENTS`` to the minimal list of required components can significantly reduce compile times.
- The ``MINIMAL_BUILD`` :ref:`build property <cmake-build-properties>` can be set to ``ON``, which acts as a shortcut to configure the ``COMPONENTS`` variable to include only the ``main`` component. This means that the build will include only the :ref:`common components <component-common-requirements>`, the ``main`` component, and all dependencies associated with it, both direct and indirect. If the ``COMPONENTS`` variable is defined while the ``MINIMAL_BUILD`` property is enabled, ``COMPONENTS`` will take precedence.

.. note::

   Certain features and configurations, such as those provided by esp_psram or espcoredump components, may not be available to your project by default if the minimal list of components is used. When using the ``COMPONENTS`` variable, ensure that all necessary components are included. Similarly, when using the ``MINIMAL_BUILD`` build property, ensure that all required components are specified in the ``REQUIRES`` or ``PRIV_REQUIRES`` argument during component registration.

.. _component-circular-dependencies:

Circular Dependencies
---------------------

It's possible for a project to contain Component A that requires (``REQUIRES`` or ``PRIV_REQUIRES``) Component B, and Component B that requires Component A. This is known as a dependency cycle or a circular dependency.

CMake will usually handle circular dependencies automatically by repeating the component library names twice on the linker command line. However this strategy doesn't always work, and the build may fail with a linker error about "Undefined reference to ...", referencing a symbol defined by one of the components inside the circular dependency. This is particularly likely if there is a large circular dependency, i.e., A > B > C > D > A.

The best solution is to restructure the components to remove the circular dependency. In most cases, a software architecture without circular dependencies has desirable properties of modularity and clean layering and will be more maintainable in the long term. However, removing circular dependencies is not always possible.

To bypass a linker error caused by a circular dependency, the simplest workaround is to increase the CMake `LINK_INTERFACE_MULTIPLICITY`_ property of one of the component libraries. This causes CMake to repeat this library and its dependencies more than two times on the linker command line.

For example:

.. code-block:: cmake

    set_property(TARGET ${COMPONENT_LIB} APPEND PROPERTY LINK_INTERFACE_MULTIPLICITY 3)

- This line should be placed after ``idf_component_register`` in the component CMakeLists.txt file.
- If possible, place this line in the component that creates the circular dependency by depending on a lot of other components. However, the line can be placed inside any component that is part of the cycle. Choosing the component that owns the source file shown in the linker error message, or the component that defines the symbol(s) mentioned in the linker error message, is a good place to start.
- Usually increasing the value to 3 (default is 2) is enough, but if this doesn't work then try increasing the number further.
- Adding this option will make the linker command line longer, and the linking stage slower.


Advanced Workaround: Undefined Symbols
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If only one or two symbols are causing a circular dependency, and all other dependencies are linear, then there is an alternative method to avoid linker errors: Specify the specific symbols required for the "reverse" dependency as undefined symbols at link time.

For example, if component A depends on component B but component B also needs to reference ``reverse_ops`` from component A (but nothing else), then you can add a line like the following to the component B CMakeLists.txt to resolve the cycle at link time:

.. code-block:: cmake

    # This symbol is provided by 'Component A' at link time
    target_link_libraries(${COMPONENT_LIB} INTERFACE "-u reverse_ops")

- The ``-u`` argument means that the linker will always include this symbol in the link, regardless of dependency ordering.
- This line should be placed after ``idf_component_register`` in the component CMakeLists.txt file.
- If 'Component B' doesn't need to access any headers of 'Component A', only link to a few symbol(s), then this line can be used instead of any ``REQUIRES`` from B to A. This further simplifies the component structure in the build system.

See the `target_link_libraries`_ documentation for more information about this CMake function.


.. _component-requirements-implementation:

Requirements in the Build System Implementation
-----------------------------------------------

- Very early in the CMake configuration process, the script ``expand_requirements.cmake`` is run. This script does a partial evaluation of all component CMakeLists.txt files and builds a graph of component requirements (this :ref:`graph may have cycles <component-circular-dependencies>`). The graph is used to generate a file ``component_depends.cmake`` in the build directory.
- The main CMake process then includes this file and uses it to determine the list of components to include in the build (internal ``BUILD_COMPONENTS`` variable). The ``BUILD_COMPONENTS`` variable is sorted so dependencies are listed first, however, as the component dependency graph has cycles this cannot be guaranteed for all components. The order should be deterministic given the same set of components and component dependencies.
- The value of ``BUILD_COMPONENTS`` is logged by CMake as "Component names: "
- Configuration is then evaluated for the components included in the build.
- Each component is included in the build normally and the CMakeLists.txt file is evaluated again to add the component libraries to the build.


Component Dependency Order
^^^^^^^^^^^^^^^^^^^^^^^^^^

The order of components in the ``BUILD_COMPONENTS`` variable determines other orderings during the build:

- Order that :ref:`project_include.cmake` files are included in the project.
- Order that the list of header paths is generated for compilation (via ``-I`` argument). (Note that for a given component's source files, only that component's dependency's header paths are passed to the compiler.)


Adding Link-Time Dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. highlight:: cmake

The ESP-IDF CMake helper function ``idf_component_add_link_dependency`` adds a link-only dependency between one component and another. In almost all cases, it is better to use the ``PRIV_REQUIRES`` feature in ``idf_component_register`` to create a dependency. However, in some cases, it's necessary to add the link-time dependency of another component to this component, i.e., the reverse order to ``PRIV_REQUIRES`` (for example: :doc:`/api-reference/peripherals/spi_flash/spi_flash_override_driver`).

To make another component depend on this component at link time::

  idf_component_add_link_dependency(FROM other_component)

Place this line after the line with ``idf_component_register``.

It's also possible to specify both components by name::

  idf_component_add_link_dependency(FROM other_component TO that_component)


.. _override_project_config:

Overriding Parts of the Project
===============================

.. _project_include.cmake:

Project_include.cmake
---------------------

For components that have build requirements that must be evaluated before any component CMakeLists files are evaluated, you can create a file called ``project_include.cmake`` in the component directory. This CMake file is included when ``project.cmake`` is evaluating the entire project.

``project_include.cmake`` files are used inside ESP-IDF, for defining project-wide build features such as ``esptool.py`` command line arguments and the ``bootloader`` "special app".

Unlike component ``CMakeLists.txt`` files, when including a ``project_include.cmake`` file the current source directory (``CMAKE_CURRENT_SOURCE_DIR`` and working directory) is the project directory. Use the variable ``COMPONENT_DIR`` for the absolute directory of the component.

Note that ``project_include.cmake`` isn't necessary for the most common component uses, such as adding include directories to the project, or ``LDFLAGS`` to the final linking step. These values can be customized via the ``CMakeLists.txt`` file itself. See `Optional Project Variables`_ for details.

``project_include.cmake`` files are included in the order given in ``BUILD_COMPONENTS`` variable (as logged by CMake). This means that a component's ``project_include.cmake`` file will be included after it's all dependencies' ``project_include.cmake`` files, unless both components are part of a dependency cycle. This is important if a ``project_include.cmake`` file relies on variables set by another component. See also :ref:`above <component-requirements-implementation>`.

Take great care when setting variables or targets in a ``project_include.cmake`` file. As the values are included in the top-level project CMake pass, they can influence or break functionality across all components!


Kconfig.projbuild
-----------------

This is an equivalent to ``project_include.cmake`` for :ref:`component-configuration` Kconfig files. If you want to include configuration options at the top level of menuconfig, rather than inside the "Component Configuration" sub-menu, then these can be defined in the Kconfig.projbuild file alongside the ``CMakeLists.txt`` file.

Take care when adding configuration values in this file, as they will be included across the entire project configuration. Where possible, it's generally better to create a Kconfig file for :ref:`component-configuration`.

For more information, see :ref:`Kconfig Files <kconfig-files>` section in the Configuration Guide.

Wrappers to Redefine or Extend Existing Functions
-------------------------------------------------

Thanks to the linker's wrap feature, it is possible to redefine or extend the behavior of an existing ESP-IDF function. To do so, you will need to provide the following CMake declaration in your project's ``CMakeLists.txt`` file:

.. code-block:: cmake

    target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=function_to_redefine")

Where ``function_to_redefine`` is the name of the function to redefine or extend. This option will let the linker replace all the calls to ``function_to_redefine`` functions in the binary libraries with calls to ``__wrap_function_to_redefine`` function. Thus, you must define this new symbol in your application.

The linker will provide a new symbol named ``__real_function_to_redefine`` which points to the former implementation of the function to redefine. It can be called from the new implementation, making it an extension of the former one.

This mechanism is shown in the example :example:`build_system/wrappers`. Check :idf_file:`examples/build_system/wrappers/README.md` for more details.


Override the Default Bootloader
-------------------------------

Thanks to the optional ``bootloader_components`` directory present in your ESP-IDF project, it is possible to override the default ESP-IDF bootloader. To do so, a new ``bootloader_components/main`` component should be defined, which will make the project directory tree look like the following:

    - myProject/
                 - CMakeLists.txt
                 - sdkconfig
                 - bootloader_components/ - main/ - CMakeLists.txt
                                                  - Kconfig
                                                  - my_bootloader.c
                 - main/       - CMakeLists.txt
                               - app_main.c

                 - build/


Here, the ``my_bootloader.c`` file becomes source code for the new bootloader, which means that it will need to perform all the required operations to set up and load the ``main`` application from flash.

It is also possible to conditionally replace the bootloader depending on a certain condition, such as the target for example. This can be achieved thanks to the ``BOOTLOADER_IGNORE_EXTRA_COMPONENT`` CMake variable. This list can be used to tell the ESP-IDF bootloader project to ignore and not compile the given components present in ``bootloader_components``. For example, if one wants to use the default bootloader for ESP32 target, then ``myProject/CMakeLists.txt`` should look like the following::

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    if(${IDF_TARGET} STREQUAL "esp32")
        set(BOOTLOADER_IGNORE_EXTRA_COMPONENT "main")
    endif()

    project(main)

It is important to note that this can also be used for any other bootloader components than ``main``. In all cases, the prefix ``bootloader_component`` must not be specified.

See :example:`custom_bootloader/bootloader_override` for an example of overriding the default bootloader.

Similarly to regular applications, it is possible to include external components, not placed in `bootloader_component`, as part of the bootloader build thanks to the build property ``BOOTLOADER_EXTRA_COMPONENT_DIRS``. It can either refer to a directory that contains several components, or refer to a single component. For example:

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    idf_build_set_property(BOOTLOADER_EXTRA_COMPONENT_DIRS "/path/to/extra/component/" APPEND)

    project(main)

See :example:`custom_bootloader/bootloader_extra_dir` for an example of adding extra components to the bootloader build.

.. _config_only_component:

Configuration-Only Components
=============================

Special components which contain no source files, only ``Kconfig.projbuild`` and ``Kconfig``, can have a one-line ``CMakeLists.txt`` file which calls the function ``idf_component_register()`` with no arguments specified. This function will include the component in the project build, but no library will be built *and* no header files will be added to any included paths.


Debugging CMake
===============

For full details about CMake_ and CMake commands, see the `CMake v3.22 documentation`_.

Some tips for debugging the ESP-IDF CMake-based build system:

- When CMake runs, it prints quite a lot of diagnostic information including lists of components and component paths.
- Running ``cmake -DDEBUG=1`` will produce more verbose diagnostic output from the IDF build system.
- Running ``cmake`` with the ``--trace`` or ``--trace-expand`` options will give a lot of information about control flow. See the `cmake command line documentation`_.

When included from a project CMakeLists file, the ``project.cmake`` file defines some utility modules and global variables and then sets ``IDF_PATH`` if it was not set in the system environment.

It also defines an overridden custom version of the built-in CMake_ ``project`` function. This function is overridden to add all of the ESP-IDF specific project functionality.


.. _warn-undefined-variables:

Warning On Undefined Variables
------------------------------

By default, the function of warnings on undefined variables is disabled.

To enable this function, we can pass the ``--warn-uninitialized`` flag to CMake_ or pass the ``--cmake-warn-uninitialized`` flag to ``idf.py`` so it will print a warning if an undefined variable is referenced in the build. This can be very useful to find buggy CMake files.

Browse the :idf_file:`/tools/cmake/project.cmake` file and supporting functions in :idf:`/tools/cmake/` for more details.


.. _component_cmakelists_example:


Example Component CMakeLists
============================

Because the build environment tries to set reasonable defaults that will work most of the time, component ``CMakeLists.txt`` can be very small or even empty (see `Minimal Component CMakeLists`_). However, overriding `preset_component_variables`_ is usually required for some functionality.

Here are some more advanced examples of component CMakeLists files.


.. _add_conditional_config:

Adding Conditional Configuration
--------------------------------

The configuration system can be used to conditionally compile some files depending on the options selected in the project configuration.

.. highlight:: none

``Kconfig``::

    config FOO_ENABLE_BAR
        bool "Enable the BAR feature."
        help
            This enables the BAR feature of the FOO component.

``CMakeLists.txt``::

    set(srcs "foo.c" "more_foo.c")

    if(CONFIG_FOO_ENABLE_BAR)
        list(APPEND srcs "bar.c")
    endif()

   idf_component_register(SRCS "${srcs}"
                        ...)

This example makes use of the CMake `if <cmake if_>`_ function and `list APPEND <cmake list_>`_ function.

This can also be used to select or stub out an implementation, as such:

``Kconfig``::

    config ENABLE_LCD_OUTPUT
        bool "Enable LCD output."
        help
            Select this if your board has an LCD.

    config ENABLE_LCD_CONSOLE
        bool "Output console text to LCD"
        depends on ENABLE_LCD_OUTPUT
        help
            Select this to output debugging output to the LCD

    config ENABLE_LCD_PLOT
        bool "Output temperature plots to LCD"
        depends on ENABLE_LCD_OUTPUT
        help
            Select this to output temperature plots

.. highlight:: cmake

``CMakeLists.txt``::

    if(CONFIG_ENABLE_LCD_OUTPUT)
       set(srcs lcd-real.c lcd-spi.c)
    else()
       set(srcs lcd-dummy.c)
    endif()

    # We need font if either console or plot is enabled
    if(CONFIG_ENABLE_LCD_CONSOLE OR CONFIG_ENABLE_LCD_PLOT)
       list(APPEND srcs "font.c")
    endif()

    idf_component_register(SRCS "${srcs}"
                        ...)


Conditions Which Depend on the Target
-------------------------------------

The current target is available to CMake files via ``IDF_TARGET`` variable.

In addition to that, if target ``xyz`` is used (``IDF_TARGET=xyz``), then Kconfig variable ``CONFIG_IDF_TARGET_XYZ`` will be set.

Note that component dependencies may depend on ``IDF_TARGET`` variable, but not on Kconfig variables. Also one can not use Kconfig variables in ``include`` statements in CMake files, but ``IDF_TARGET`` can be used in such context.


Source Code Generation
----------------------

Some components will have a situation where a source file isn't supplied with the component itself but has to be generated from another file. Say our component has a header file that consists of the converted binary data of a BMP file, converted using a hypothetical tool called bmp2h. The header file is then included in as C source file called graphics_lib.c::

    add_custom_command(OUTPUT logo.h
         COMMAND bmp2h -i ${COMPONENT_DIR}/logo.bmp -o log.h
         DEPENDS ${COMPONENT_DIR}/logo.bmp
         VERBATIM)

    add_custom_target(logo DEPENDS logo.h)
    add_dependencies(${COMPONENT_LIB} logo)

    set_property(DIRECTORY "${COMPONENT_DIR}" APPEND PROPERTY
         ADDITIONAL_CLEAN_FILES logo.h)

This answer is adapted from the `CMake FAQ entry <cmake faq generated files_>`_, which contains some other examples that will also work with ESP-IDF builds.

In this example, logo.h will be generated in the current directory (the build directory) while logo.bmp comes with the component and resides under the component path. Because logo.h is a generated file, it should be cleaned when the project is cleaned. For this reason, it is added to the `ADDITIONAL_CLEAN_FILES`_ property.

.. note::

   If generating files as part of the project CMakeLists.txt file, not a component CMakeLists.txt, then use build property ``PROJECT_DIR`` instead of ``${COMPONENT_DIR}`` and ``${PROJECT_NAME}.elf`` instead of ``${COMPONENT_LIB}``.)

If a a source file from another component included ``logo.h``, then ``add_dependencies`` would need to be called to add a dependency between the two components, to ensure that the component source files were always compiled in the correct order.


.. _cmake_embed_data:

Embedding Binary Data
---------------------

Sometimes you have a file with some binary or text data that you'd like to make available to your component, but you don't want to reformat the file as a C source.

You can specify argument ``EMBED_FILES`` in the component registration, giving space-delimited names of the files to embed::

  idf_component_register(...
                         EMBED_FILES server_root_cert.der)

Or if the file is a string, you can use the variable ``EMBED_TXTFILES``. This will embed the contents of the text file as a null-terminated string::

  idf_component_register(...
                         EMBED_TXTFILES server_root_cert.pem)

.. highlight:: c

The file's contents will be added to the .rodata section in flash, and are available via symbol names as follows::

  extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
  extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

The names are generated from the full name of the file, as given in ``EMBED_FILES``. Characters /, ., etc. are replaced with underscores. The _binary prefix in the symbol name is added by objcopy and is the same for both text and binary files.

.. highlight:: cmake

To embed a file into a project, rather than a component, you can call the function ``target_add_binary_data`` like this::

  target_add_binary_data(myproject.elf "main/data.bin" TEXT)

Place this line after the ``project()`` line in your project CMakeLists.txt file. Replace ``myproject.elf`` with your project name. The final argument can be ``TEXT`` to embed a null-terminated string, or ``BINARY`` to embed the content as-is.

For an example of using this technique, see the "main" component of the file_serving example :example_file:`protocols/http_server/file_serving/main/CMakeLists.txt` - two files are loaded at build time and linked into the firmware.

.. highlight:: cmake

It is also possible to embed a generated file::

  add_custom_command(OUTPUT my_processed_file.bin
                    COMMAND my_process_file_cmd my_unprocessed_file.bin)
  target_add_binary_data(my_target "my_processed_file.bin" BINARY)

In the example above, ``my_processed_file.bin`` is generated from ``my_unprocessed_file.bin`` through some command ``my_process_file_cmd``, then embedded into the target.

To specify a dependence on a target, use the ``DEPENDS`` argument::

  add_custom_target(my_process COMMAND ...)
  target_add_binary_data(my_target "my_embed_file.bin" BINARY DEPENDS my_process)

The ``DEPENDS`` argument to ``target_add_binary_data`` ensures that the target executes first.


Code and Data Placements
------------------------

ESP-IDF has a feature called linker script generation that enables components to define where its code and data will be placed in memory through linker fragment files. These files are processed by the build system, and is used to augment the linker script used for linking app binary. See :doc:`Linker Script Generation <linker-script-generation>` for a quick start guide as well as a detailed discussion of the mechanism.


.. _component-build-full-override:

Fully Overriding the Component Build Process
--------------------------------------------

.. highlight:: cmake

Obviously, there are cases where all these recipes are insufficient for a certain component, for example when the component is basically a wrapper around another third-party component not originally intended to be compiled under this build system. In that case, it's possible to forego the ESP-IDF build system entirely by using a CMake feature called ExternalProject_. Example component CMakeLists::

  # External build process for quirc, runs in source dir and
  # produces libquirc.a
  externalproject_add(quirc_build
      PREFIX ${COMPONENT_DIR}
      SOURCE_DIR ${COMPONENT_DIR}/quirc
      CONFIGURE_COMMAND ""
      BUILD_IN_SOURCE 1
      BUILD_COMMAND make CC=${CMAKE_C_COMPILER} libquirc.a
      INSTALL_COMMAND ""
      )

   # Add libquirc.a to the build process
   add_library(quirc STATIC IMPORTED GLOBAL)
   add_dependencies(quirc quirc_build)

   set_target_properties(quirc PROPERTIES IMPORTED_LOCATION
        ${COMPONENT_DIR}/quirc/libquirc.a)
   set_target_properties(quirc PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
        ${COMPONENT_DIR}/quirc/lib)

   set_directory_properties( PROPERTIES ADDITIONAL_CLEAN_FILES
        "${COMPONENT_DIR}/quirc/libquirc.a")

(The above CMakeLists.txt can be used to create a component named ``quirc`` that builds the quirc_ project using its own Makefile.)

- ``externalproject_add`` defines an external build system.

  - ``SOURCE_DIR``, ``CONFIGURE_COMMAND``, ``BUILD_COMMAND`` and ``INSTALL_COMMAND`` should always be set. ``CONFIGURE_COMMAND`` can be set to an empty string if the build system has no "configure" step. ``INSTALL_COMMAND`` will generally be empty for ESP-IDF builds.
  - Setting ``BUILD_IN_SOURCE`` means the build directory is the same as the source directory. Otherwise, you can set ``BUILD_DIR``.
  - Consult the ExternalProject_ documentation for more details about ``externalproject_add()``

- The second set of commands adds a library target, which points to the "imported" library file built by the external system. Some properties need to be set in order to add include directories and tell CMake where this file is.
- Finally, the generated library is added to `ADDITIONAL_CLEAN_FILES`_. This means ``make clean`` will delete this library. (Note that the other object files from the build won't be deleted.)

.. only:: esp32

   .. note:: When using an external build process with PSRAM, remember to add ``-mfix-esp32-psram-cache-issue`` to the C compiler arguments. See :ref:`CONFIG_SPIRAM_CACHE_WORKAROUND` for details of this flag.


.. _ADDITIONAL_CLEAN_FILES_note:

ExternalProject Dependencies and Clean Builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

CMake has some unusual behavior around external project builds:

- `ADDITIONAL_CLEAN_FILES`_ only works when "make" or "ninja" is used as the build system. If an IDE build system is used, it won't delete these files when cleaning.
- However, the ExternalProject_ configure & build commands will *always* be re-run after a clean is run.
- Therefore, there are two alternative recommended ways to configure the external build command:

    1. Have the external ``BUILD_COMMAND`` run a full clean compile of all sources. The build command will be run if any of the dependencies passed to ``externalproject_add`` with ``DEPENDS`` have changed, or if this is a clean build (ie any of ``idf.py clean``, ``ninja clean``, or ``make clean`` was run.)
    2. Have the external ``BUILD_COMMAND`` be an incremental build command. Pass the parameter ``BUILD_ALWAYS 1`` to ``externalproject_add``. This means the external project will be built each time a build is run, regardless of dependencies. This is only recommended if the external project has correct incremental build behavior, and doesn't take too long to run.

The best of these approaches for building an external project will depend on the project itself, its build system, and whether you anticipate needing to frequently recompile the project.


.. _custom-sdkconfig-defaults:

Custom Sdkconfig Defaults
=========================

.. note::

  For more detailed information about ``sdkconfig.defaults`` file, please visit :ref:`sdkconfig.defaults file <sdkconfig-defaults-file>` in Project Configuration section.

For example projects or other projects where you don't want to specify a full sdkconfig configuration, but you do want to override some key values from the ESP-IDF defaults, it is possible to create a file ``sdkconfig.defaults`` in the project directory. This file will be used when creating a new config from scratch, or when any new config value hasn't yet been set in the ``sdkconfig`` file.

To override the name of this file or to specify multiple files, set the ``SDKCONFIG_DEFAULTS`` environment variable or set ``SDKCONFIG_DEFAULTS`` in top-level ``CMakeLists.txt``. File names that are not specified as full paths are resolved relative to current project's directory.

When specifying multiple files, use a semicolon as the list separator. Files listed first will be applied first. If a particular key is defined in multiple files, the definition in the latter file will override definitions from former files.

Some of the IDF examples include a ``sdkconfig.ci`` file. This is part of the continuous integration (CI) test framework and is ignored by the normal build process.


Target-dependent Sdkconfig Defaults
-----------------------------------

If and only if an ``sdkconfig.defaults`` file exists, the build system will also attempt to load defaults from an ``sdkconfig.defaults.TARGET_NAME`` file, where ``TARGET_NAME`` is the value of ``IDF_TARGET``. For example, for ``esp32`` target, default settings will be taken from ``sdkconfig.defaults`` first, and then from ``sdkconfig.defaults.esp32``. If there are no generic default settings, an empty ``sdkconfig.defaults`` still needs to be created if the build system should recognize any additional target-dependent ``sdkconfig.defaults.TARGET_NAME`` files.

If ``SDKCONFIG_DEFAULTS`` is used to override the name of defaults file/files, the name of target-specific defaults file will be derived from ``SDKCONFIG_DEFAULTS`` value/values using the rule above. When there are multiple files in ``SDKCONFIG_DEFAULTS``, target-specific file will be applied right after the file bringing it in, before all latter files in ``SDKCONFIG_DEFAULTS``

For example, if ``SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig_devkit1"``, and there is a file ``sdkconfig.defaults.esp32`` in the same folder, then the files will be applied in the following order: (1) sdkconfig.defaults (2) sdkconfig.defaults.esp32 (3) sdkconfig_devkit1.

You can find more detailed information on how the project configuration works in the :ref:`Project Configuration Guide <project-configuration-guide>`. In the :ref:`Configuration Files Structure and Relationships <configuration-structure>`, you can find lower-level information about the configuration files.

.. _flash_parameters:

Flash Arguments
===============

There are some scenarios that we want to flash the target board without IDF. For this case we want to save the built binaries, esptool.py and esptool write_flash arguments. It's simple to write a script to save binaries and esptool.py.

After running a project build, the build directory contains binary output files (``.bin`` files) for the project and also the following flashing data files:

- ``flash_project_args`` contains arguments to flash the entire project (app, bootloader, partition table, PHY data if this is configured).
- ``flash_app_args`` contains arguments to flash only the app.
- ``flash_bootloader_args`` contains arguments to flash only the bootloader.

.. highlight:: bash

You can pass any of these flasher argument files to ``esptool.py`` as follows::

  python esptool.py --chip esp32 write_flash @build/flash_project_args

Alternatively, it is possible to manually copy the parameters from the argument file and pass them on the command line.

The build directory also contains a generated file ``flasher_args.json`` which contains project flash information, in JSON format. This file is used by ``idf.py`` and can also be used by other tools which need information about the project build.


Building the Bootloader
=======================

The bootloader is a special "subproject" inside :idf:`/components/bootloader/subproject`. It has its own project CMakeLists.txt file and builds separate .ELF and .BIN files to the main project. However, it shares its configuration and build directory with the main project.

The subproject is inserted as an external project from the top-level project, by the file :idf_file:`/components/bootloader/project_include.cmake`. The main build process runs CMake for the subproject, which includes discovering components (a subset of the main components) and generating a bootloader-specific config (derived from the main ``sdkconfig``).


.. _write-pure-component:

Writing Pure CMake Components
=============================

The ESP-IDF build system "wraps" CMake with the concept of "components", and helper functions to automatically integrate these components into a project build.

However, underneath the concept of "components" is a full CMake build system. It is also possible to make a component which is pure CMake.

.. highlight:: cmake

Here is an example minimal "pure CMake" component CMakeLists file for a component named ``json``::

  add_library(json STATIC
  cJSON/cJSON.c
  cJSON/cJSON_Utils.c)

  target_include_directories(json PUBLIC cJSON)

- This is actually an equivalent declaration to the IDF ``json`` component :idf_file:`/components/json/CMakeLists.txt`.
- This file is quite simple as there are not a lot of source files. For components with a large number of files, the globbing behavior of ESP-IDF's component logic can make the component CMakeLists style simpler.)
- Any time a component adds a library target with the component name, the ESP-IDF build system will automatically add this to the build, expose public include directories, etc. If a component wants to add a library target with a different name, dependencies will need to be added manually via CMake commands.


Using Third-Party CMake Projects with Components
================================================

CMake is used for a lot of open-source C and C++ projects — code that users can tap into for their applications. One of the benefits of having a CMake build system is the ability to import these third-party projects, sometimes even without modification! This allows for users to be able to get functionality that may not yet be provided by a component, or use another library for the same functionality.

.. highlight:: cmake

Importing a library might look like this for a hypothetical library ``foo`` to be used in the ``main`` component::

  # Register the component
  idf_component_register(...)

  # Set values of hypothetical variables that control the build of `foo`
  set(FOO_BUILD_STATIC OFF)
  set(FOO_BUILD_TESTS OFF)

  # Create and import the library targets
  add_subdirectory(foo)

  # Publicly link `foo` to `main` component
  target_link_libraries(main PUBLIC foo)

For an actual example, take a look at :example:`build_system/cmake/import_lib`. Take note that what needs to be done in order to import the library may vary. It is recommended to read up on the library's documentation for instructions on how to import it from other projects. Studying the library's CMakeLists.txt and build structure can also be helpful.

It is also possible to wrap a third-party library to be used as a component in this manner. For example, the :component:`mbedtls` component is a wrapper for Espressif's fork of `mbedtls <https://github.com/Mbed-TLS/mbedtls>`_. See its :component_file:`component CMakeLists.txt <mbedtls/CMakeLists.txt>`.

The CMake variable ``ESP_PLATFORM`` is set to 1 whenever the ESP-IDF build system is being used. Tests such as ``if (ESP_PLATFORM)`` can be used in generic CMake code if special IDF-specific logic is required.


Using ESP-IDF Components from External Libraries
------------------------------------------------

The above example assumes that the external library ``foo`` (or ``tinyxml`` in the case of the ``import_lib`` example) doesn't need to use any ESP-IDF APIs apart from common APIs such as libc, libstdc++, etc. If the external library needs to use APIs provided by other ESP-IDF components, this needs to be specified in the external CMakeLists.txt file by adding a dependency on the library target ``idf::<componentname>``.

For example, in the ``foo/CMakeLists.txt`` file::

  add_library(foo bar.c fizz.cpp buzz.cpp)

  if(ESP_PLATFORM)
    # On ESP-IDF, bar.c needs to include esp_flash.h from the spi_flash component
    target_link_libraries(foo PRIVATE idf::spi_flash)
  endif()


Using Prebuilt Libraries with Components
========================================

.. highlight:: cmake

Another possibility is that you have a prebuilt static library (``.a`` file), built by some other build process.

The ESP-IDF build system provides a utility function ``add_prebuilt_library`` for users to be able to easily import and use prebuilt libraries::

  add_prebuilt_library(target_name lib_path [REQUIRES req1 req2 ...] [PRIV_REQUIRES req1 req2 ...])

where:

- ``target_name``- name that can be used to reference the imported library, such as when linking to other targets
- ``lib_path``- path to prebuilt library; may be an absolute or relative path to the component directory

Optional arguments ``REQUIRES`` and ``PRIV_REQUIRES`` specify dependency on other components. These have the same meaning as the arguments for ``idf_component_register``.

Take note that the prebuilt library must have been compiled for the same target as the consuming project. Configuration relevant to the prebuilt library must also match. If not paid attention to, these two factors may contribute to subtle bugs in the app.

For an example, take a look at :example:`build_system/cmake/import_prebuilt`.


Using ESP-IDF in Custom CMake Projects
======================================

ESP-IDF provides a template CMake project for easily creating an application. However, in some instances the user might already have an existing CMake project or may want to create a custom one. In these cases it is desirable to be able to consume IDF components as libraries to be linked to the user's targets (libraries/executables).

It is possible to do so by using the :ref:`build system APIs provided <cmake_buildsystem_api>` by :idf_file:`tools/cmake/idf.cmake`. For example:

.. code-block:: cmake

  cmake_minimum_required(VERSION 3.22)
  project(my_custom_app C)

  # Include CMake file that provides ESP-IDF CMake build system APIs.
  include($ENV{IDF_PATH}/tools/cmake/idf.cmake)

  # Include ESP-IDF components in the build, may be thought as an equivalent of
  # add_subdirectory() but with some additional processing and magic for ESP-IDF build
  # specific build processes.
  idf_build_process(esp32)

  # Create the project executable and plainly link the esp_libc component to it using
  # its alias, idf::esp_libc.
  add_executable(${CMAKE_PROJECT_NAME}.elf main.c)
  target_link_libraries(${CMAKE_PROJECT_NAME}.elf idf::esp_libc)

  # Let the build system know what the project executable is to attach more targets, dependencies, etc.
  idf_build_executable(${CMAKE_PROJECT_NAME}.elf)

The example in :example:`build_system/cmake/idf_as_lib` demonstrates the creation of an application equivalent to :example:`hello world application <get-started/hello_world>` using a custom CMake project.

.. only:: esp32

   .. note:: The IDF build system can only set compiler flags for source files that it builds. When an external CMakeLists.txt file is used and PSRAM is enabled, remember to add ``-mfix-esp32-psram-cache-issue`` to the C compiler arguments. See :ref:`CONFIG_SPIRAM_CACHE_WORKAROUND` for details of this flag.


.. _cmake_buildsystem_api:

ESP-IDF CMake Build System API
==============================

Idf-build-commands
------------------

.. code-block:: none

  idf_build_get_property(var property [GENERATOR_EXPRESSION])

Retrieve a :ref:`build property <cmake-build-properties>` *property* and store it in *var* accessible from the current scope. Specifying *GENERATOR_EXPRESSION* will retrieve the generator expression string for that property, instead of the actual value, which can be used with CMake commands that support generator expressions.

.. code-block:: none

  idf_build_set_property(property val [APPEND])

Set a :ref:`build property <cmake-build-properties>` *property* with value *val*. Specifying *APPEND* will append the specified value to the current value of the property. If the property does not previously exist or it is currently empty, the specified value becomes the first element/member instead.

.. code-block:: none

  idf_build_component(component_dir [component_source])

Present a directory *component_dir* that contains a component to the build system. Relative paths are converted to absolute paths with respect to current directory.

An optional *component_source* argument can be specified to indicate the source of the component. (default: "project_components")

This argument determines the overriding priority for components with the same name. For detailed information, see :ref:`cmake-components-same-name`.

This argument supports the following values (from highest to lowest priority):

- "project_components" - project components
- "project_extra_components" - components from ``EXTRA_COMPONENT_DIRS``
- "project_managed_components" - custom project dependencies managed by the IDF Component Manager
- "idf_components" - ESP-IDF built-in components, typically under :idf:`/components`

For instance, if a component named "json" is present as both "idf_components", and "project_components", the component as "project_components" takes precedence over the one as "idf_components".

.. warning::

    All calls to this command must be performed before `idf_build_process`. This command does not guarantee that the component will be processed during build (see the `COMPONENTS` argument description for `idf_build_process`).

.. code-block:: none

  idf_build_process(target
                    [PROJECT_DIR project_dir]
                    [PROJECT_VER project_ver]
                    [PROJECT_NAME project_name]
                    [SDKCONFIG sdkconfig]
                    [SDKCONFIG_DEFAULTS sdkconfig_defaults]
                    [BUILD_DIR build_dir]
                    [COMPONENTS component1 component2 ...])

Performs the bulk of the behind-the-scenes magic for including ESP-IDF components such as component configuration, libraries creation, dependency expansion and resolution. Among these functions, perhaps the most important from a user's perspective is the libraries creation by calling each component's ``idf_component_register``. This command creates the libraries for each component, which are accessible using aliases in the form idf::*component_name*.
These aliases can be used to link the components to the user's own targets, either libraries or executables.

The call requires the target chip to be specified with *target* argument. Optional arguments for the call include:

- PROJECT_DIR - directory of the project; defaults to CMAKE_SOURCE_DIR
- PROJECT_NAME - name of the project; defaults to CMAKE_PROJECT_NAME
- PROJECT_VER - version/revision of the project; defaults to "1"
- SDKCONFIG - output path of generated sdkconfig file; defaults to PROJECT_DIR/sdkconfig or CMAKE_SOURCE_DIR/sdkconfig depending if PROJECT_DIR is set
- SDKCONFIG_DEFAULTS - list of files containing default config to use in the build (list must contain full paths); defaults to empty. For each value *filename* in the list, the config from file *filename.target*, if it exists, is also loaded.
- BUILD_DIR - directory to place ESP-IDF build-related artifacts, such as generated binaries, text files, components; defaults to CMAKE_BINARY_DIR
- COMPONENTS - select components to process among the components known by the build system (added via `idf_build_component`). This argument is used to trim the build.
  Other components are automatically added if they are required in the dependency chain, i.e., the public and private requirements of the components in this list are automatically added, and in turn the public and private requirements of those requirements, so on and so forth. If not specified, all components known to the build system are processed.

.. code-block:: none

  idf_build_executable(executable)

Specify the executable *executable* for ESP-IDF build. This attaches additional targets such as dependencies related to flashing, generating additional binary files, etc. Should be called after ``idf_build_process``.

.. code-block:: none

  idf_build_get_config(var config [GENERATOR_EXPRESSION])

Get the value of the specified config. Much like build properties, specifying *GENERATOR_EXPRESSION* will retrieve the generator expression string for that config, instead of the actual value, which can be used with CMake commands that support generator expressions. Actual config values are only known after call to ``idf_build_process``, however.

.. code-block:: none

  idf_build_add_post_elf_dependency(elf_filename dep_target)

Register a dependency that must run after the ELF is linked (post-ELF) and before the binary image is generated. This is useful when a component needs to post‑process the ELF in place prior to ``elf2image`` execution (for example, inserting metadata, stripping sections, or generating additional symbol files). The dependency target ``dep_target`` must be a valid CMake target. If your rule reads or modifies the ELF, declare the ELF file as a ``DEPENDS`` of your custom command.

.. important::

   When creating post‑ELF steps, ensure the build graph remains acyclic:

   - Do not make the ELF itself the output of your custom command. Produce a separate output (for example, ``app.elf.post``, ``app.elf.symbols``, or a simple marker file).
   - If you must modify the ELF in place, also produce an additional output file and update its timestamp to be newer than the ELF after modification (for example, using ``cmake -E touch``). This ensures the output file has a newer timestamp than the modified ELF, so CMake considers the rule satisfied and won't re-run it on subsequent builds.

   Following these rules ensures the post‑ELF hook runs in the intended order without triggering infinite rebuild loops.

Example:

.. code-block:: cmake

    # Create a custom command to process the ELF file after linking
    idf_build_get_property(elf_target EXECUTABLE GENERATOR_EXPRESSION)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.stripped_marker"
        COMMAND ${CMAKE_OBJCOPY} --strip-debug
                "$<TARGET_FILE:$<GENEX_EVAL:${elf_target}>>"
        COMMAND ${CMAKE_COMMAND} -E touch
                "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.stripped_marker"
        DEPENDS "$<TARGET_FILE:$<GENEX_EVAL:${elf_target}>>"
    )

    # Wrap it in a custom target
    add_custom_target(strip_elf DEPENDS
        "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.stripped_marker"
    )

    # Register it to run after the ELF is linked but before the BIN is generated
    idf_build_add_post_elf_dependency("${CMAKE_PROJECT_NAME}.elf" strip_elf)

.. code-block:: none

  idf_build_get_post_elf_dependencies(elf_filename out_var)

Retrieve the list of post-ELF dependencies registered for the given ELF file and store it in ``out_var``.


.. _cmake-build-properties:

Idf-build-properties
--------------------

These are properties that describe the build. Values of build properties can be retrieved by using the build command ``idf_build_get_property``. For example, to get the Python interpreter used for the build:

.. code-block:: cmake

  idf_build_get_property(python PYTHON)
  message(STATUS "The Python interpreter is: ${python}")

- BUILD_DIR - build directory; set from ``idf_build_process`` BUILD_DIR argument
- BUILD_COMPONENTS - list of components included in the build; set by ``idf_build_process``
- BUILD_COMPONENT_ALIASES - list of library alias of components included in the build; set by ``idf_build_process``
- C_COMPILE_OPTIONS - compile options applied to all components' C source files
- COMPILE_OPTIONS - compile options applied to all components' source files, regardless of it being C or C++
- COMPILE_DEFINITIONS - compile definitions applied to all component source files
- CXX_COMPILE_OPTIONS - compile options applied to all components' C++ source files
- DEPENDENCIES_LOCK - lock file path used in component manager. The default value is `dependencies.lock` under the project path.
- EXECUTABLE - project executable; set by call to ``idf_build_executable``
- EXECUTABLE_NAME - name of project executable without extension; set by call to ``idf_build_executable``
- EXECUTABLE_DIR - path containing the output executable
- IDF_COMPONENT_MANAGER - the component manager is enabled by default, but if this property is set to ``0`` it was disabled by the IDF_COMPONENT_MANAGER environment variable
- IDF_PATH - ESP-IDF path; set from IDF_PATH environment variable, if not, inferred from the location of ``idf.cmake``
- IDF_TARGET - target chip for the build; set from the required target argument for ``idf_build_process``
- IDF_VER - ESP-IDF version; set from either a version file or the Git revision of the IDF_PATH repository
- INCLUDE_DIRECTORIES - include directories for all component source files
- KCONFIGS - list of Kconfig files found in components in build; set by ``idf_build_process``
- KCONFIG_PROJBUILDS - list of Kconfig.projbuild files found in components in build; set by ``idf_build_process``
- MINIMAL_BUILD - perform a minimal build by including only the "common" components required by all other components, along with the components that are direct or transitive dependencies only of the ``main`` component. By default, this property is disabled (set to ``OFF``), but it can be enabled by setting it to ``ON``.
- PROJECT_NAME - name of the project; set from ``idf_build_process`` PROJECT_NAME argument
- PROJECT_DIR - directory of the project; set from ``idf_build_process`` PROJECT_DIR argument
- PROJECT_VER - version of the project; set from ``idf_build_process`` PROJECT_VER argument
- PYTHON - Python interpreter used for the build; set from PYTHON environment variable if available, if not "python" is used
- SDKCONFIG - full path to output config file; set from ``idf_build_process`` SDKCONFIG argument
- SDKCONFIG_DEFAULTS - list of files containing default config to use in the build; set from ``idf_build_process`` SDKCONFIG_DEFAULTS argument
- SDKCONFIG_HEADER - full path to C/C++ header file containing component configuration; set by ``idf_build_process``
- SDKCONFIG_CMAKE - full path to CMake file containing component configuration; set by ``idf_build_process``
- SDKCONFIG_JSON - full path to JSON file containing component configuration; set by ``idf_build_process``
- SDKCONFIG_JSON_MENUS - full path to JSON file containing config menus; set by ``idf_build_process``


Idf-component-commands
----------------------

.. code-block:: none

  idf_component_get_property(var component property [GENERATOR_EXPRESSION])

Retrieve a specified *component*'s :ref:`component property <cmake-component-properties>`, *property* and store it in *var* accessible from the current scope. Specifying *GENERATOR_EXPRESSION* will retrieve the generator expression string for that property, instead of the actual value, which can be used with CMake commands that support generator expressions.

.. code-block:: none

  idf_component_set_property(component property val [APPEND])

Set a specified *component*'s :ref:`component property <cmake-component-properties>`, *property* with value *val*. Specifying *APPEND* will append the specified value to the current value of the property. If the property does not previously exist or it is currently empty, the specified value becomes the first element/member instead.

.. _cmake-component-register:

.. code-block:: none

  idf_component_register([[SRCS src1 src2 ...] | [[SRC_DIRS dir1 dir2 ...] [EXCLUDE_SRCS src1 src2 ...]]
                         [INCLUDE_DIRS dir1 dir2 ...]
                         [PRIV_INCLUDE_DIRS dir1 dir2 ...]
                         [REQUIRES component1 component2 ...]
                         [PRIV_REQUIRES component1 component2 ...]
                         [LDFRAGMENTS ldfragment1 ldfragment2 ...]
                         [REQUIRED_IDF_TARGETS target1 target2 ...]
                         [EMBED_FILES file1 file2 ...]
                         [EMBED_TXTFILES file1 file2 ...]
                         [KCONFIG kconfig]
                         [KCONFIG_PROJBUILD kconfig_projbuild]
                         [WHOLE_ARCHIVE])

Register a component to the build system. Much like the ``project()`` CMake command, this should be called from the component's CMakeLists.txt directly (not through a function or macro) and is recommended to be called before any other command. Here are some guidelines on what commands can **not** be called before ``idf_component_register``:

  - commands that are not valid in CMake script mode
  - custom commands defined in project_include.cmake
  - build system API commands except ``idf_build_get_property``; although consider whether the property may not have been set yet

Commands that set and operate on variables are generally okay to call before ``idf_component_register``.

The arguments for ``idf_component_register`` include:

  - SRCS - component source files used for creating a static library for the component; if not specified, component is a treated as a config-only component and an interface library is created instead.
  - SRC_DIRS, EXCLUDE_SRCS - used to glob source files (.c, .cpp, .S) by specifying directories, instead of specifying source files manually via SRCS. Note that this is subject to the :ref:`limitations of globbing in CMake <cmake-file-globbing>`. Source files specified in EXCLUDE_SRCS are removed from the globbed files.
  - INCLUDE_DIRS - paths, relative to the component directory, which will be added to the include search path for all other components which require the current component
  - PRIV_INCLUDE_DIRS - directory paths, must be relative to the component directory, which will be added to the include search path for this component's source files only
  - REQUIRES - public component requirements for the component
  - PRIV_REQUIRES - private component requirements for the component; ignored on config-only components
  - LDFRAGMENTS - component linker fragment files
  - REQUIRED_IDF_TARGETS - specify the only target the component supports
  - KCONFIG - override the default Kconfig file
  - KCONFIG_PROJBUILD - override the default Kconfig.projbuild file
  - WHOLE_ARCHIVE - if specified, the component library is surrounded by ``-Wl,--whole-archive``, ``-Wl,--no-whole-archive`` when linked. This has the same effect as setting ``WHOLE_ARCHIVE`` component property.

The following are used for :ref:`embedding data into the component <cmake_embed_data>`, and is considered as source files when determining if a component is config-only. This means that even if the component does not specify source files, a static library is still created internally for the component if it specifies either:

  - EMBED_FILES - binary files to be embedded in the component
  - EMBED_TXTFILES - text files to be embedded in the component


.. _cmake-component-properties:

Idf-component-properties
------------------------

These are properties that describe a component. Values of component properties can be retrieved by using the build command ``idf_component_get_property``. For example, to get the directory of the ``freertos`` component:

.. code-block:: cmake

  idf_component_get_property(dir freertos COMPONENT_DIR)
  message(STATUS "The 'freertos' component directory is: ${dir}")

- COMPONENT_ALIAS - alias for COMPONENT_LIB used for linking the component to external targets; set by ``idf_build_component`` and alias library itself is created by ``idf_component_register``
- COMPONENT_DIR - component directory; set by ``idf_build_component``
- COMPONENT_OVERRIDEN_DIR - contains the directory of the original component if :ref:`this component overrides another component <cmake-components-same-name>`
- COMPONENT_LIB - name for created component static/interface library; set by ``idf_build_component`` and library itself is created by ``idf_component_register``
- COMPONENT_NAME - name of the component; set by ``idf_build_component`` based on the component directory name
- COMPONENT_TYPE - type of the component, whether LIBRARY or CONFIG_ONLY. A component is of type LIBRARY if it specifies source files or embeds a file
- COMPONENT_SOURCE - source of the component, one of "idf_components", "project_managed_components", "project_components", "project_extra_components". This is used to determine the override precedence of components with the same name.
- EMBED_FILES - list of files to embed in component; set from ``idf_component_register`` EMBED_FILES argument
- EMBED_TXTFILES - list of text files to embed in component; set from ``idf_component_register`` EMBED_TXTFILES argument
- INCLUDE_DIRS - list of component include directories; set from ``idf_component_register`` INCLUDE_DIRS argument
- KCONFIG - component Kconfig file; set by ``idf_build_component``
- KCONFIG_PROJBUILD - component Kconfig.projbuild; set by ``idf_build_component``
- LDFRAGMENTS - list of component linker fragment files; set from ``idf_component_register`` LDFRAGMENTS argument
- MANAGED_PRIV_REQUIRES - list of private component dependencies added by the IDF component manager from dependencies in ``idf_component.yml`` manifest file
- MANAGED_REQUIRES - list of public component dependencies added by the IDF component manager from dependencies in ``idf_component.yml`` manifest file
- PRIV_INCLUDE_DIRS - list of component private include directories; set from ``idf_component_register`` PRIV_INCLUDE_DIRS on components of type LIBRARY
- PRIV_REQUIRES - list of private component dependencies; set from value of ``idf_component_register`` PRIV_REQUIRES argument and dependencies in ``idf_component.yml`` manifest file
- REQUIRED_IDF_TARGETS - list of targets the component supports; set from ``idf_component_register`` REQUIRED_IDF_TARGETS argument
- REQUIRES - list of public component dependencies; set from value of ``idf_component_register`` REQUIRES argument and dependencies in ``idf_component.yml`` manifest file
- SRCS - list of component source files; set from SRCS or SRC_DIRS/EXCLUDE_SRCS argument of ``idf_component_register``
- WHOLE_ARCHIVE - if this property is set to ``TRUE`` (or any boolean "true" CMake value: 1, ``ON``, ``YES``, ``Y``), the component library is surrounded by ``-Wl,--whole-archive``, ``-Wl,--no-whole-archive`` when linked. This can be used to force the linker to include every object file into the executable, even if the object file doesn't resolve any references from the rest of the application. This is commonly used when a component contains plugins or modules which rely on link-time registration. This property is ``FALSE`` by default. It can be set to ``TRUE`` from the component CMakeLists.txt file.


.. _cmake-file-globbing:

File Globbing & Incremental Builds
==================================

.. highlight:: cmake

The preferred way to include source files in an ESP-IDF component is to list them manually via SRCS argument to ``idf_component_register``::

  idf_component_register(SRCS library/a.c library/b.c platform/platform.c
                         ...)

This preference reflects the `CMake best practice <https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1/>`_ of manually listing source files. This could, however, be inconvenient when there are lots of source files to add to the build. The ESP-IDF build system provides an alternative way for specifying source files using ``SRC_DIRS``::

  idf_component_register(SRC_DIRS library platform
                         ...)

This uses globbing behind the scenes to find source files in the specified directories. Be aware, however, that if a new source file is added and this method is used, then CMake won't know to automatically re-run and this file won't be added to the build.

The trade-off is acceptable when you're adding the file yourself, because you can trigger a clean build or run ``idf.py reconfigure`` to manually re-run CMake_. However, the problem gets harder when you share your project with others who may check out a new version using a source control tool like Git...

For components which are part of ESP-IDF, we use a third party Git CMake integration module (:idf_file:`/tools/cmake/third_party/GetGitRevisionDescription.cmake`) which automatically re-runs CMake any time the repository commit changes. This means if you check out a new ESP-IDF version, CMake will automatically rerun.

For project components (not part of ESP-IDF), there are a few different options:

- If keeping your project file in Git, ESP-IDF will automatically track the Git revision and re-run CMake if the revision changes.
- If some components are kept in a third git repository (not the project repository or ESP-IDF repository), you can add a call to the ``git_describe`` function in a component CMakeLists file in order to automatically trigger re-runs of CMake when the Git revision changes.
- If not using Git, remember to manually run ``idf.py reconfigure`` whenever a source file may change.
- To avoid this problem entirely, use ``SRCS`` argument to ``idf_component_register`` to list all source files in project components.

The best option will depend on your particular project and its users.


.. _build_system_metadata:

Build System Metadata
=====================

For integration into IDEs and other build systems, when CMake runs the build process generates a number of metadata files in the ``build/`` directory. To regenerate these files, run ``cmake`` or ``idf.py reconfigure`` (or any other ``idf.py`` build command).

- ``compile_commands.json`` is a standard format JSON file which describes every source file which is compiled in the project. A CMake feature generates this file, and many IDEs know how to parse it.
- ``project_description.json`` contains some general information about the ESP-IDF project, configured paths, etc.
- ``flasher_args.json`` contains esptool.py arguments to flash the project's binary files. There are also ``flash_*_args`` files which can be used directly with esptool.py. See `Flash arguments`_.
- ``CMakeCache.txt`` is the CMake cache file which contains other information about the CMake process, toolchain, etc.
- ``config/sdkconfig.json`` is a JSON-formatted version of the project configuration values.
- ``config/kconfig_menus.json`` is a JSON-formatted version of the menus shown in menuconfig, for use in external IDE UIs.


JSON Configuration Server
-------------------------

A tool called ``kconfserver`` is provided to allow IDEs to easily integrate with the configuration system logic. ``kconfserver`` is designed to run in the background and interact with a calling process by reading and writing JSON over process stdin & stdout.

You can run ``kconfserver`` from a project via ``idf.py confserver`` or ``ninja kconfserver``, or a similar target triggered from a different build generator.

For more information about ``kconfserver``, see the `esp-idf-kconfig documentation <https://docs.espressif.com/projects/esp-idf-kconfig/en/latest/kconfserver/index.html>`_.


Build System Internals
======================

Build Scripts
-------------

The listfiles for the ESP-IDF build system reside in :idf:`/tools/cmake`. The modules which implement core build system functionality are as follows:

    - build.cmake - Build related commands i.e., build initialization, retrieving/setting build properties, build processing.
    - component.cmake - Component related commands i.e., adding components, retrieving/setting component properties, registering components.
    - kconfig.cmake - Generation of configuration files (sdkconfig, sdkconfig.h, sdkconfig.cmake, etc.) from Kconfig files.
    - ldgen.cmake - Generation  of  final linker script from linker fragment files.
    - target.cmake - Setting build target and toolchain file.
    - utilities.cmake - Miscellaneous helper commands.

 Aside from these files, there are two other important CMake scripts in :idf:`/tools/cmake`:

    - idf.cmake - Sets up the build and includes the core modules listed above. Included in CMake projects in order to access ESP-IDF build system functionality.
    - project.cmake - Includes ``idf.cmake`` and provides a custom ``project()`` command that takes care of all the heavy lifting of building an executable. Included in the top-level CMakeLists.txt of standard ESP-IDF projects.

The rest of the files in :idf:`/tools/cmake` are support or third-party scripts used in the build process.


Build Process
-------------

This section describes the standard ESP-IDF application build process. The build process can be broken down roughly into four phases:

.. blockdiag::
    :scale: 100%
    :caption: ESP-IDF Build System Process
    :align: center

    blockdiag idf-build-system-process {
        Initialization -> Enumeration
        Enumeration -> Processing
        Processing -> Finalization
    }


Initialization
^^^^^^^^^^^^^^

This phase sets up necessary parameters for the build.

    - Upon inclusion of ``idf.cmake`` in ``project.cmake``, the following steps are performed:
        - Set ``IDF_PATH`` from environment variable or inferred from path to ``project.cmake`` included in the top-level CMakeLists.txt.
        - Add :idf:`/tools/cmake` to ``CMAKE_MODULE_PATH`` and include core modules plus the various helper/third-party scripts.
        - Set build tools/executables such as default Python interpreter.
        - Get ESP-IDF git revision and store as ``IDF_VER``.
        - Set global build specifications i.e., compile options, compile definitions, include directories for all components in the build.
        - Add components in :idf:`components` to the build.
    - The initial part of the custom ``project()`` command performs the following steps:
        - Set ``IDF_TARGET`` from environment variable or CMake cache and the corresponding ``CMAKE_TOOLCHAIN_FILE`` to be used.
        - Add components in ``EXTRA_COMPONENT_DIRS`` to the build.
        - Prepare arguments for calling command ``idf_build_process()`` from variables such as ``COMPONENTS``/``EXCLUDE_COMPONENTS``, ``SDKCONFIG``, ``SDKCONFIG_DEFAULTS``.

  The call to ``idf_build_process()`` command marks the end of this phase.


Enumeration
^^^^^^^^^^^
  This phase builds a final list of components to be processed in the build, and is performed in the first half of ``idf_build_process()``.

    - Retrieve each component's public and private requirements. A child process is created which executes each component's CMakeLists.txt in script mode. The values of ``idf_component_register`` REQUIRES and PRIV_REQUIRES argument is returned to the parent build process. This is called early expansion. The variable ``CMAKE_BUILD_EARLY_EXPANSION`` is defined during this step.
    - Recursively include components based on public and private requirements.
    - Unless IDF Component Manager is disabled, it is called to resolve the dependencies of the components:
      - Looks for manifests and dependencies contained in the project.
      - Starts the version solving process to resolve the dependencies of the components.
      - When the version solving process succeeds, the IDF Component Manager downloads dependencies, integrates them into the build, and creates a ``dependencies.lock`` file that contains a list of the exact versions of the dependencies installed by the IDF Component Manager.


Processing
^^^^^^^^^^

  This phase processes the components in the build, and is the second half of ``idf_build_process()``.

  - Load project configuration from sdkconfig file and generate an sdkconfig.cmake and sdkconfig.h header. These define configuration variables/macros that are accessible from the build scripts and C/C++ source/header files, respectively.
  - Include each component's ``project_include.cmake``.
  - Add each component as a subdirectory, processing its CMakeLists.txt. The component CMakeLists.txt calls the registration command, ``idf_component_register`` which adds source files, include directories, creates component library, links dependencies, etc.


Finalization
^^^^^^^^^^^^
  This phase is everything after ``idf_build_process()``.

  - Create executable and link the component libraries to it.
  - Generate project metadata files such as project_description.json and display relevant information about the project built.

Browse :idf_file:`/tools/cmake/project.cmake` for more details.


.. _migrating_from_make:

Migrating from ESP-IDF GNU Make System
======================================

Some aspects of the CMake-based ESP-IDF build system are very similar to the older GNU Make-based system. The developer needs to provide values the include directories, source files etc. There is a syntactical difference, however, as the developer needs to pass these as arguments to the registration command, ``idf_component_register``.


Automatic Conversion Tool
-------------------------

An automatic project conversion tool is available in `tools/cmake/convert_to_cmake.py` in ESP-IDF v4.x releases. The script was removed in v5.0 because of its `make` build system dependency.


No Longer Available in CMake
----------------------------

Some features are significantly different or removed in the CMake-based system. The following variables no longer exist in the CMake-based build system:

- ``COMPONENT_BUILD_DIR``: Use ``CMAKE_CURRENT_BINARY_DIR`` instead.
- ``COMPONENT_LIBRARY``: Defaulted to ``$(COMPONENT_NAME).a``, but the library name could be overridden by the component. The name of the component library can no longer be overridden by the component.
- ``CC``, ``LD``, ``AR``, ``OBJCOPY``: Full paths to each tool from the gcc xtensa cross-toolchain. Use ``CMAKE_C_COMPILER``, ``CMAKE_C_LINK_EXECUTABLE``, ``CMAKE_OBJCOPY``, etc instead. `Full list here <cmake language variables_>`_.
- ``HOSTCC``, ``HOSTLD``, ``HOSTAR``: Full names of each tool from the host native toolchain. These are no longer provided, external projects should detect any required host toolchain manually.
- ``COMPONENT_ADD_LDFLAGS``: Used to override linker flags. Use the CMake `target_link_libraries`_ command instead.
- ``COMPONENT_ADD_LINKER_DEPS``: List of files that linking should depend on. `target_link_libraries`_ will usually infer these dependencies automatically. For linker scripts, use the provided custom CMake function ``target_linker_scripts``.
- ``COMPONENT_SUBMODULES``: No longer used, the build system will automatically enumerate all submodules in the ESP-IDF repository.
- ``COMPONENT_EXTRA_INCLUDES``: Used to be an alternative to ``COMPONENT_PRIV_INCLUDEDIRS`` for absolute paths. Use ``PRIV_INCLUDE_DIRS`` argument to ``idf_component_register`` for all cases now (can be relative or absolute).
- ``COMPONENT_OBJS``: Previously, component sources could be specified as a list of object files. Now they can be specified as a list of source files via ``SRCS`` argument to `idf_component_register`.
- ``COMPONENT_OBJEXCLUDE``: Has been replaced with ``EXCLUDE_SRCS`` argument to ``idf_component_register``. Specify source files (as absolute paths or relative to component directory), instead.
- ``COMPONENT_EXTRA_CLEAN``: Set property ``ADDITIONAL_CLEAN_FILES`` instead but note :ref:`CMake has some restrictions around this functionality <ADDITIONAL_CLEAN_FILES_note>`.
- ``COMPONENT_OWNBUILDTARGET`` & ``COMPONENT_OWNCLEANTARGET``: Use CMake `ExternalProject`_ instead. See :ref:`component-build-full-override` for full details.
- ``COMPONENT_CONFIG_ONLY``: Call ``idf_component_register`` without any arguments instead. See `Configuration-Only Components`_.
- ``CFLAGS``, ``CPPFLAGS``, ``CXXFLAGS``: Use equivalent CMake commands instead. See `Controlling Component Compilation`_.


No Default Values
-----------------

Unlike in the legacy Make-based build system, the following have no default values:

- Source directories (``COMPONENT_SRCDIRS`` variable in Make, ``SRC_DIRS`` argument to ``idf_component_register`` in CMake)
- Include directories (``COMPONENT_ADD_INCLUDEDIRS`` variable in Make, ``INCLUDE_DIRS`` argument to ``idf_component_register`` in CMake)


No Longer Necessary
-------------------

- In the legacy Make-based build system, it is required to also set ``COMPONENT_SRCDIRS`` if ``COMPONENT_SRCS`` is set. In CMake, the equivalent is not necessary i.e., specifying ``SRC_DIRS`` to ``idf_component_register`` if ``SRCS`` is also specified (in fact, ``SRCS`` is ignored if ``SRC_DIRS`` is specified).


Flashing from Make
------------------

``make flash`` and similar targets still work to build and flash. However, project ``sdkconfig`` no longer specifies serial port and baud rate. Environment variables can be used to override these. See :ref:`flash-with-ninja-or-make` for more details.

Application Examples
--------------------

- :example:`build_system/wrappers` demonstrates how to use a linker feature to redefine or override any public function in both ESP-IDF and the bootloader, allowing modification or extension of a function's default behavior.

- :example:`custom_bootloader/bootloader_override` demonstrates how to override the second stage bootloader from a regular project, providing a custom bootloader that prints an extra message on startup, with the ability to conditionally override the bootloader based on certain conditions like target-dependency or Kconfig options.

- :example:`build_system/cmake/import_lib` demonstrates how to import and use third-party libraries using ExternalProject CMake module.

- :example:`build_system/cmake/import_prebuilt` demonstrates how to import a prebuilt static library into the ESP-IDF build system, build a component with dependencies, and link it to the main component, ultimately outputting the current running partition.

- :example:`build_system/cmake/idf_as_lib` demonstrates the creation of an application equivalent to :example:`hello world application <get-started/hello_world>` using a custom CMake project.

- :example:`build_system/cmake/multi_config` demonstrates how to build multiple configurations of a single application from a single codebase, it is useful for creating binaries for multiple similar products.

- :example:`build_system/cmake/plugins` demonstrates features of the ESP-IDF build system related to link time registration of plugins, allowing you to add multiple implementations of a certain feature without the need to make the application aware of all these implementations.

.. _esp-idf-template: https://github.com/espressif/esp-idf-template
.. _cmake: https://cmake.org
.. _ninja: https://ninja-build.org
.. _esptool.py: https://github.com/espressif/esptool/#readme
.. _CMake v3.22 documentation: https://cmake.org/cmake/help/v3.22/index.html
.. _cmake command line documentation: https://cmake.org/cmake/help/v3.22/manual/cmake.1.html#options
.. _cmake add_library: https://cmake.org/cmake/help/v3.22/command/add_library.html
.. _cmake if: https://cmake.org/cmake/help/v3.22/command/if.html
.. _cmake list: https://cmake.org/cmake/help/v3.22/command/list.html
.. _cmake project: https://cmake.org/cmake/help/v3.22/command/project.html
.. _cmake set: https://cmake.org/cmake/help/v3.22/command/set.html
.. _cmake string: https://cmake.org/cmake/help/v3.22/command/string.html
.. _cmake faq generated files: https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#how-can-i-generate-a-source-file-during-the-build
.. _ADDITIONAL_CLEAN_FILES: https://cmake.org/cmake/help/v3.22/prop_dir/ADDITIONAL_CLEAN_FILES.html
.. _ExternalProject: https://cmake.org/cmake/help/v3.22/module/ExternalProject.html
.. _cmake language variables: https://cmake.org/cmake/help/v3.22/manual/cmake-variables.7.html#variables-for-languages
.. _set_source_files_properties: https://cmake.org/cmake/help/v3.22/command/set_source_files_properties.html
.. _target_compile_options: https://cmake.org/cmake/help/v3.22/command/target_compile_options.html
.. _target_link_libraries: https://cmake.org/cmake/help/v3.22/command/target_link_libraries.html#command:target_link_libraries
.. _cmake_toolchain_file: https://cmake.org/cmake/help/v3.22/variable/CMAKE_TOOLCHAIN_FILE.html
.. _LINK_INTERFACE_MULTIPLICITY: https://cmake.org/cmake/help/v3.22/prop_tgt/LINK_INTERFACE_MULTIPLICITY.html
.. _quirc: https://github.com/dlbeer/quirc
.. _pyenv: https://github.com/pyenv/pyenv#readme
.. _virtualenv: https://virtualenv.pypa.io/en/stable/
````

## File: docs/en/api-guides/c.rst
````
C Support
===========

:link_to_translation:`zh_CN:[中文]`

ESP-IDF is primarily written in C and provides C APIs. ESP-IDF can use one of the following C Standard Library implementations:

- `Newlib <https://sourceware.org/newlib/>`_ (default)
- `Picolibc <https://keithp.com/picolibc/>`_ (enabled with :ref:`CONFIG_LIBC_PICOLIBC<CONFIG_LIBC_PICOLIBC>` Kconfig option)

The Newlib version is specified in :component_file:`esp_libc/sbom.yml`.

In general, all C features supported by the compiler (currently GCC) can be used in ESP-IDF, unless otherwise noted in :ref:`unsupported_c_features` below.

.. _c_version:

C Version
---------

**GNU dialect of ISO C23** (``--std=gnu23``) is the current default C version in ESP-IDF.

To compile the source code of a certain component using a different language standard, set the desired compiler flag in the component's ``CMakeLists.txt`` file:

.. code-block:: cmake

    idf_component_register( ... )
    target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu11)

If the public header files of the component also need to be compiled with the same language standard, replace the flag ``PRIVATE`` with ``PUBLIC``.

.. _unsupported_c_features:

Unsupported C Features
----------------------

The following features are not supported in ESP-IDF.

Nested Function Pointers
^^^^^^^^^^^^^^^^^^^^^^^^

The **GNU dialect of ISO C23** supports `nested functions <https://gcc.gnu.org/onlinedocs/gcc/Nested-Functions.html>`_. However, ESP-IDF does not support referencing nested functions as pointers. This is due to the fact that the GCC compiler generates a `trampoline <https://gcc.gnu.org/onlinedocs/gccint/Trampolines.html>`_ (i.e., small piece of executable code) on the stack when a pointer to a nested function is referenced. ESP-IDF does not permit executing code from a stack, thus use of pointers to nested functions is not supported.
````

## File: docs/en/api-guides/coexist.rst
````
RF Coexistence
==================

:link_to_translation:`zh_CN:[中文]`

Overview
---------------

ESP boards now support three modules: Bluetooth (BT & BLE), IEEE 802.15.4 (Thread / Zigbee), and Wi-Fi. Each type of board has only one 2.4 GHz ISM band RF module, shared by two or three modules. Consequently, a module cannot receive or transmit data while another module is engaged in data transmission or reception. In such scenarios, {IDF_TARGET_NAME} employs the time-division multiplexing method to manage the reception and transmission of packets.


Supported Coexistence Scenario for {IDF_TARGET_NAME}
---------------------------------------------------------------------

.. only:: SOC_WIFI_SUPPORTED and SOC_BLE_SUPPORTED

  .. table:: Supported Features of Wi-Fi and BLE Coexistence

      +-------+--------+-----------+-----+------------+----------+
      |                            |BLE                          |
      +                            +-----+------------+----------+
      |                            |Scan |Advertising |Connected |
      +-------+--------+-----------+-----+------------+----------+
      | Wi-Fi |STA     |Scan       |Y    |Y           |Y         |
      +       +        +-----------+-----+------------+----------+
      |       |        |Connecting |Y    |Y           |Y         |
      +       +        +-----------+-----+------------+----------+
      |       |        |Connected  |Y    |Y           |Y         |
      +       +--------+-----------+-----+------------+----------+
      |       |SOFTAP  |TX Beacon  |Y    |Y           |Y         |
      +       +        +-----------+-----+------------+----------+
      |       |        |Connecting |C1   |C1          |C1        |
      +       +        +-----------+-----+------------+----------+
      |       |        |Connected  |C1   |C1          |C1        |
      +       +--------+-----------+-----+------------+----------+
      |       |Sniffer |RX         |C1   |C1          |C1        |
      +       +--------+-----------+-----+------------+----------+
      |       |ESP-NOW |RX         |S    |S           |S         |
      +       +        +-----------+-----+------------+----------+
      |       |        |TX         |Y    |Y           |Y         |
      +-------+--------+-----------+-----+------------+----------+


.. only:: SOC_WIFI_SUPPORTED and SOC_BT_CLASSIC_SUPPORTED

  .. table:: Supported Features of Wi-Fi and Classic Bluetooth (BT) Coexistence

      +-------+--------+-----------+--------+-------------+-----+----------+-----------+
      |                            |BR/EDR                                             |
      +                            +--------+-------------+-----+----------+-----------+
      |                            |Inquiry |Inquiry scan |Page |Page scan | Connected |
      +-------+--------+-----------+--------+-------------+-----+----------+-----------+
      | Wi-Fi |STA     |Scan       |Y       |Y            |Y    |Y         |Y          |
      +       +        +-----------+--------+-------------+-----+----------+-----------+
      |       |        |Connecting |Y       |Y            |Y    |Y         |Y          |
      +       +        +-----------+--------+-------------+-----+----------+-----------+
      |       |        |Connected  |Y       |Y            |Y    |Y         |Y          |
      +       +--------+-----------+--------+-------------+-----+----------+-----------+
      |       |SOFTAP  |TX Beacon  |Y       |Y            |Y    |Y         |Y          |
      +       +        +-----------+--------+-------------+-----+----------+-----------+
      |       |        |Connecting |C1      |C1           |C1   |C1        |C1         |
      +       +        +-----------+--------+-------------+-----+----------+-----------+
      |       |        |Connected  |C1      |C1           |C1   |C1        |C1         |
      +       +--------+-----------+--------+-------------+-----+----------+-----------+
      |       |Sniffer |RX         |C1      |C1           |C1   |C1        |C1         |
      +       +--------+-----------+--------+-------------+-----+----------+-----------+
      |       |ESP-NOW |RX         |S       |S            |S    |S         |S          |
      +       +        +-----------+--------+-------------+-----+----------+-----------+
      |       |        |TX         |Y       |Y            |Y    |Y         |Y          |
      +-------+--------+-----------+--------+-------------+-----+----------+-----------+

.. only:: SOC_WIFI_SUPPORTED and SOC_IEEE802154_SUPPORTED

  .. table:: Supported Features of Wi-Fi and IEEE 802.15.4 (Thread / Zigbee) Coexistence

      +-------+--------+-----------+--------+---------+-----------+
      |                            |Thread / Zigbee               |
      +                            +--------+---------+-----------+
      |                            |Scan    |Router   |End Device |
      +-------+--------+-----------+--------+---------+-----------+
      | Wi-Fi |STA     |Scan       |C1      |C1       |Y          |
      +       +        +-----------+--------+---------+-----------+
      |       |        |Connecting |C1      |C1       |Y          |
      +       +        +-----------+--------+---------+-----------+
      |       |        |Connected  |C1      |C1       |Y          |
      +       +--------+-----------+--------+---------+-----------+
      |       |SOFTAP  |TX Beacon  |Y       |X        |Y          |
      +       +        +-----------+--------+---------+-----------+
      |       |        |Connecting |C1      |X        |C1         |
      +       +        +-----------+--------+---------+-----------+
      |       |        |Connected  |C1      |X        |C1         |
      +       +--------+-----------+--------+---------+-----------+
      |       |Sniffer |RX         |C1      |X        |C1         |
      +-------+--------+-----------+--------+---------+-----------+

.. only:: SOC_BLE_SUPPORTED and SOC_IEEE802154_SUPPORTED

  .. table:: Supported Features of IEEE 802.15.4 (Thread / Zigbee) and BLE Coexistence

      +-----------------+-------------+-----+------------+----------+
      |                               |BLE                          |
      +                               +-----+------------+----------+
      |                               |Scan |Advertising |Connected |
      +-----------------+-------------+-----+------------+----------+
      | Thread / Zigbee |Scan         |X    |Y           |Y         |
      +                 +-------------+-----+------------+----------+
      |                 |Router       |X    |Y           |Y         |
      +                 +-------------+-----+------------+----------+
      |                 |End Device   |C1   |Y           |Y         |
      +-----------------+-------------+-----+------------+----------+

.. note::

  .. list::

    - Y: supported and the performance is stable
    - C1: supported but the performance is unstable
    - X: not supported
    :SOC_WIFI_SUPPORTED: - S: supported and the performance is stable in STA mode, otherwise not supported

.. only:: SOC_IEEE802154_SUPPORTED

  .. note::

    Routers in Thread and Zigbee networks maintain unsynchronized links with their neighbors, requiring continuous signal reception. With only a single RF path, increased Wi-Fi or BLE traffic may lead to higher packet loss rates for Thread and Zigbee communications.

    To build a Wi-Fi based Thread Border Router or Zigbee Gateway product, we recommend using a dual-SoC solution (e.g., ESP32-S3 + ESP32-H2) with separate antennas. This setup enables simultaneous reception of Wi-Fi and 802.15.4 signals, ensuring optimal performance.


Coexistence Mechanism and Policy
------------------------------------------------

Coexistence Mechanism
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The RF resource allocation mechanism is based on priority. As shown below, Wi-Fi, Bluetooth and 802.15.4 modules request RF resources from the coexistence module, and the coexistence module decides who will use the RF resource based on their priority.

.. blockdiag::
    :scale: 100%
    :caption: Coexistence Mechanism
    :align: center

    blockdiag {

      # global attributes
      node_height = 60;
      node_width = 120;
      span_width = 100;
      span_height = 60;
      default_shape = roundedbox;
      default_group_color = none;

      # node labels
       Wi-Fi [shape = box];
       Bluetooth [shape = box];
       802.15.4 [shape = box];
       Coexistence [shape = box, label = 'Coexistence module'];
       RF [shape = box, label = 'RF module'];

      # node connections
       Wi-Fi -> Coexistence;
       Bluetooth  -> Coexistence;
       802.15.4  -> Coexistence;
       Coexistence -> RF;
    }


.. _coexist_policy:

Coexistence Policy
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: SOC_WIFI_SUPPORTED and SOC_BT_SUPPORTED

  Coexistence Period and Time Slice
  """"""""""""""""""""""""""""""""""""""""

  .. only:: SOC_BLE_SUPPORTED and SOC_BT_CLASSIC_SUPPORTED

    Wi-Fi, BT, and BLE have their fixed time slice to use the RF. A coexistence period is divided into 3 time slices in the order of Wi-Fi, BT, and BLE. In the Wi-Fi slice, Wi-Fi's request to the coexistence arbitration module will have higher priority. Similarly, BT/BLE can enjoy higher priority at their own time slices. The duration of the coexistence period and the proportion of each time slice are divided into four categories according to the Wi-Fi status:


  .. only:: not SOC_BT_CLASSIC_SUPPORTED

    Wi-Fi and BLE have their fixed time slice to use the RF. In the Wi-Fi time slice, Wi-Fi will send a higher priority request to the coexistence arbitration module. Similarly, BLE can enjoy higher priority at their own time slice. The duration of the coexistence period and the proportion of each time slice are divided into four categories according to the Wi-Fi status:

  .. list::

    :SOC_BLE_SUPPORTED and SOC_BT_CLASSIC_SUPPORTED: 1) IDLE status: the coexistence of BT and BLE is controlled by Bluetooth module.
    :not SOC_BT_CLASSIC_SUPPORTED: 1) IDLE status: RF module is controlled by Bluetooth module.
    #) CONNECTED status: the coexistence period starts at the Target Beacon Transmission Time (TBTT) and is more than 100 ms.
    #) SCAN status: Wi-Fi slice and coexistence period are longer than in the CONNECTED status. To ensure Bluetooth performance, the Bluetooth time slice will also be adjusted accordingly.
    #) CONNECTING status: Wi-Fi slice is longer than in the CONNECTED status. To ensure Bluetooth performance, the Bluetooth time slice will also be adjusted accordingly.


  According to the coexistence logic, different coexistence periods and time slice strategies will be selected based on the Wi-Fi and Bluetooth usage scenarios. A Coexistence policy corresponding to a certain usage scenarios is called a "coexistence scheme". For example, the scenario of Wi-Fi CONNECTED and BLE CONNECTED has a corresponding coexistence scheme. In this scheme, the time slices of Wi-Fi and BLE in a coexistence period each account for 50%. The time allocation is shown in the following figure:

  .. figure:: ../../_static/coexist_wifi_connected_and_ble_connected_time_slice.png
      :align: center
      :alt: Time Slice Under the Status of Wi-Fi CONNECTED and BLE CONNECTED
      :figclass: align-center

      Time Slice Under the Status of Wi-Fi CONNECTED and BLE CONNECTED

.. only:: SOC_IEEE802154_SUPPORTED

    The IEEE 802.15.4 module requests RF resources based on pre-assigned priorities. Normal receive operations are assigned the lowest priority, meaning Wi-Fi and BLE will take over the RF whenever needed, while 802.15.4 can only receive during the remaining time. Other 802.15.4 operations, such as transmitting or receiving ACKs and transmitting or receiving at given time, are assigned higher priorities. However, their access to RF ultimately depends on the priorities of Wi-Fi and BLE operations at that moment.

.. only:: SOC_WIFI_SUPPORTED and SOC_BT_SUPPORTED

    Dynamic Priority
    """"""""""""""""""""""""""""

    The coexistence module assigns varying priorities to different statuses of each module, and these priorities are dynamic. For example, in every N BLE Advertising events, there is always one event with high priority. If a high-priority BLE Advertising event occurs within the Wi-Fi time slice, the right to use the RF may be preempted by BLE.

.. only:: SOC_WIFI_SUPPORTED

    Wi-Fi Connectionless Modules Coexistence
    """"""""""""""""""""""""""""""""""""""""""""""""""""""""

    To some extent, some combinations of connectionless power-saving parameters `Window` and `Interval` would lead to extra Wi-Fi priority request out of Wi-Fi time slice. It`s for obtaining RF resources at coexistence for customized parameters, while leading to impact on Bluetooth performance.

    If connectionless power-saving parameters are configured with default values, the coexistence module would perform in stable mode and the behaviour above would not happen. So please configure Wi-Fi connectionless power-saving parameters to default values unless you have plenty of coexistence performance tests for customized parameters.

    Please refer to :ref:`connectionless module power save <connectionless-module-power-save>` to get more detail.


How to Use the Coexistence Feature
--------------------------------------

Coexistence API
^^^^^^^^^^^^^^^^^^^^^^^^^^^

For most coexistence cases, {IDF_TARGET_NAME} will switch the coexistence status automatically without calling API. However, {IDF_TARGET_NAME} provides two APIs for the coexistence of BLE MESH and Wi-Fi. When the status of BLE MESH changes, call :code:`esp_coex_status_bit_clear` to clear the previous status first and then call :code:`esp_coex_status_bit_set` to set the current status.


BLE MESH Coexistence Status
""""""""""""""""""""""""""""""""""

As the firmware of Wi-Fi and Bluetooth are not aware of the current scenario of the upper layer application, some coexistence schemes require application code to call the coexistence API to take effect. The application layer needs to pass the working status of BLE MESH to the coexistence module for selecting the coexistence scheme.

  - ESP_COEX_BLE_ST_MESH_CONFIG: network is provisioning
  - ESP_COEX_BLE_ST_MESH_TRAFFIC: data is transmitting
  - ESP_COEX_BLE_ST_MESH_STANDBY: in idle status with no significant data interaction


Coexistence API Error Codes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

All coexistence APIs have custom return values, i.e., error codes. These error codes can be categorized as:

  - No error. For example, the return value ESP_OK signifies the API returned successfully.
  - Recoverable errors. For example, the return value ESP_ERR_INVALID_ARG signifies API parameter errors.


Setting Coexistence Compile-time Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list::

  - After writing the coexistence program, you must check  :ref:`CONFIG_ESP_COEX_SW_COEXIST_ENABLE` option through menuconfig to open coexistence configuration on software, otherwise the coexistence function mentioned above cannot be used.
  :esp32: - To ensure better communication performance of Wi-Fi and Bluetooth in the case of coexistence, run the task of the Wi-Fi protocol stack, the task of the Bluetooth Controller and Host protocol stack on different CPUs. You can use :ref:`CONFIG_BTDM_CTRL_PINNED_TO_CORE_CHOICE` and :ref:`CONFIG_BT_BLUEDROID_PINNED_TO_CORE_CHOICE` (or :ref:`CONFIG_BT_NIMBLE_PINNED_TO_CORE_CHOICE`) to put the tasks of the Bluetooth controller and the host protocol stack on the same CPU, and then use :ref:`CONFIG_ESP_WIFI_TASK_CORE_ID` to place the task of the Wi-Fi protocol stack on another CPU.
  :esp32s3: - To ensure better communication performance of Wi-Fi and Bluetooth in the case of coexistence, run the task of the Wi-Fi protocol stack, the task of the Bluetooth Controller and Host protocol stack on different CPUs. You can use :ref:`CONFIG_BT_CTRL_PINNED_TO_CORE_CHOICE` and :ref:`CONFIG_BT_BLUEDROID_PINNED_TO_CORE_CHOICE` (or :ref:`CONFIG_BT_NIMBLE_PINNED_TO_CORE_CHOICE` ）to put the tasks of the Bluetooth controller and the host protocol stack on the same CPU, and then use :ref:`CONFIG_ESP_WIFI_TASK_CORE_ID` to place the task of the Wi-Fi protocol stack on another CPU.
  :esp32: - In the case of coexistence, BLE SCAN may be interrupted by Wi-Fi and Wi-Fi releases RF resources before the end of the current BLE scan window. In order to make BLE acquire RF resources again within the current scan window, you can check the FULL SCAN configuration option through :ref:`CONFIG_BTDM_CTRL_FULL_SCAN_SUPPORTED`.
  :esp32c3 or esp32s3: - When using LE Coded PHY during a BLE connection, to avoid affecting Wi-Fi performance due to the long duration of Bluetooth packets, you can select `BT_CTRL_COEX_PHY_CODED_TX_RX_TLIM_EN` in the sub-options of :ref:`CONFIG_BT_CTRL_COEX_PHY_CODED_TX_RX_TLIM` to limit the maximum time of TX/RX.
  :esp32c2 or esp32c6: - When using LE Coded PHY during a BLE connection, to avoid affecting Wi-Fi performance due to the long duration of Bluetooth packets, you can select `BT_LE_COEX_PHY_CODED_TX_RX_TLIM_EN` in the sub-options of :ref:`CONFIG_BT_LE_COEX_PHY_CODED_TX_RX_TLIM` to limit the maximum time of TX/RX.
  :SOC_BT_SUPPORTED or SOC_WIFI_SUPPORTED: - You can reduce the memory consumption by configuring the following options on menuconfig.

    .. only:: SOC_BT_SUPPORTED

      - :ref:`CONFIG_BT_BLE_DYNAMIC_ENV_MEMORY`: enable the configuration of dynamic memory for Bluetooth protocol stack.

    .. only:: SOC_WIFI_SUPPORTED

      - :ref:`CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM`: reduce the number of Wi-Fi static RX buffers.
      - :ref:`CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM`: reduce the number of Wi-Fi dynamic RX buffers.
      - :ref:`CONFIG_ESP_WIFI_TX_BUFFER`: enable the configuration of dynamic allocation TX buffers.
      - :ref:`CONFIG_ESP_WIFI_DYNAMIC_TX_BUFFER_NUM`: reduce the number of Wi-Fi dynamic TX buffers.
      - :ref:`CONFIG_ESP_WIFI_TX_BA_WIN`: reduce the number of Wi-Fi Block Ack TX windows.
      - :ref:`CONFIG_ESP_WIFI_RX_BA_WIN`: reduce the number of Wi-Fi Block Ack RX windows.
      - :ref:`CONFIG_ESP_WIFI_MGMT_SBUF_NUM`: reduce the number of Wi-Fi Management Short Buffer.
      - :ref:`CONFIG_ESP_WIFI_RX_IRAM_OPT`: turning off this configuration option will reduce the IRAM memory by approximately 17 KB.
      - :ref:`CONFIG_LWIP_TCP_SND_BUF_DEFAULT`: reduce the default TX buffer size for TCP sockets.
      - :ref:`CONFIG_LWIP_TCP_WND_DEFAULT`:  reduce the default size of the RX window for TCP sockets.
      - :ref:`CONFIG_LWIP_TCP_RECVMBOX_SIZE`: reduce the size of the TCP receive mailbox. Receive mailbox buffers data within active connections and handles data flow during connections。
      - :ref:`CONFIG_LWIP_TCP_ACCEPTMBOX_SIZE`: reduce the size of the TCP accept mailbox. Accept mailbox queues incoming connection requests and manages the initiation of new connections.
      - :ref:`CONFIG_LWIP_UDP_RECVMBOX_SIZE`: reduce the size of the UDP receive mailbox.
      - :ref:`CONFIG_LWIP_TCPIP_RECVMBOX_SIZE`: reduce the size of TCPIP task receive mailbox.


.. note::

    As the coexistence configuration option relies on the presence of any two enabled modules, please ensure that both modules are activated before configuring any coexistence features.
````

## File: docs/en/api-guides/core_dump_internals.rst
````
Anatomy of Core Dump Image
--------------------------

:link_to_translation:`zh_CN:[中文]`

Core dump files are generated in ELF format, which provides comprehensive information regarding the software's state at the moment the crash occurs, including CPU registers and memory contents.

The memory state embeds a snapshot of all tasks mapped in the memory space of the program. The CPU state contains register values when the core dump has been generated. The core dump file uses a subset of the ELF structures to register this information.

Loadable ELF segments are used to store the process' memory state, while ELF notes (``ELF.PT_NOTE``) are used to store the process' metadata (e.g., PID, registers, signal etc). In particular, the CPU's status is stored in a note with a special name and type (``CORE``, ``NT_PRSTATUS type``).

Here is an overview of the core dump layout:

.. figure:: ../../_static/core_dump_format_elf.png
    :align: center
    :alt: Core Dump ELF Image Format
    :figclass: align-center

    Core Dump ELF Image Format

.. note::

    The format of the image file shown in the above pictures represents the current version of the image and can be changed in future releases.


Overview of Implementation
--------------------------

The figure below describes some basic aspects related to the implementation of the core dump:

.. figure:: ../../_static/core_dump_impl.png
    :align: center
    :alt: Core Dump Implementation Overview
    :figclass: align-center

    Core Dump Implementation Overview

.. note::

    The diagram above hides some details and represents the current implementation of the core dump which can be changed later.
````

## File: docs/en/api-guides/core_dump.rst
````
Core Dump
=========

:link_to_translation:`zh_CN:[中文]`

Overview
--------

A core dump is a set of software state information that is automatically saved by the panic handler when a fatal error occurs. Core dumps are useful for conducting post-mortem analysis of the software's state at the moment of failure. ESP-IDF provides support for generating core dumps.

A core dump contains snapshots of all tasks in the system at the moment of failure, where each snapshot includes a task's control block (TCB) and stack. By analyzing the task snapshots, it is possible to find out what task, at what instruction (line of code), and what call stack of that task lead to the crash. It is also possible to dump the contents of variables on demand, provided those variables are assigned special core dump attributes.

Core dump data is saved to a core dump file according to a particular format, see :doc:`Core dump internals <core_dump_internals>` for more details. However, ESP-IDF's ``idf.py`` command provides special subcommands to decode and analyze the core dump file.


Configurations
--------------

.. note::

    The ``Core dump`` configuration options are available only if the ``espcoredump`` component is included in the build. To include ``Core dump`` into your project, add the ``espcoredump`` component as a dependency in either ``REQUIRES`` or ``PRIV_REQUIRES`` when registering your component with ``idf_component_register``.

Destination
^^^^^^^^^^^

The :ref:`CONFIG_ESP_COREDUMP_TO_FLASH_OR_UART` option enables or disables core dump, and selects the core dump destination if enabled. When a crash occurs, the generated core dump file can either be saved to flash, or output to a connected host over UART.


Format & Size
^^^^^^^^^^^^^

Core dump files are generated in ELF format, which contains extended features and allows comprehensive information regarding erroneous tasks and crashed software to be saved. The ELF format is flexible and can be extended in future revisions to save additional information.

The :ref:`CONFIG_ESP_COREDUMP_MAX_TASKS_NUM` option configures the number of task snapshots saved by the core dump. Crashed task registers and the stack are always saved, regardless of this configuration option. Other tasks are included in order of their priority (starting with the highest-priority ready task).

Data Integrity Check
^^^^^^^^^^^^^^^^^^^^

Core dump files include a SHA256 checksum that verifies the integrity of the file and ensure it has not been corrupted. The SHA256 hash algorithm provides a high probability of detecting corruption, including multiple-bit errors.

Reserved Stack Size
^^^^^^^^^^^^^^^^^^^

Core dump routines run from a separate stack due to core dump itself needing to parse and save all other task stacks. The :ref:`CONFIG_ESP_COREDUMP_STACK_SIZE` option controls the size of the core dump's stack in number of bytes.

Setting this option to 0 bytes will cause the core dump routines to run from the ISR stack, thus saving a bit of memory. Setting the option greater than zero will cause a separate stack to be instantiated.

.. note::

   If a separate stack is used, the recommended stack size should be larger than 1300 bytes to ensure that the core dump routines themselves do not cause a stack overflow.


.. only:: not esp32c5

    Core Dump Memory Regions
    ^^^^^^^^^^^^^^^^^^^^^^^^

    By default, core dumps typically save CPU registers, tasks data and summary of the panic reason. When the :ref:`CONFIG_ESP_COREDUMP_CAPTURE_DRAM` option is selected, ``.bss`` and ``.data`` sections and ``heap`` data will also be part of the dump.

    For a better debugging experience, it is recommended to dump these sections. However, this will result in a larger coredump file. The required additional storage space may vary based on the amount of DRAM the application uses.

    .. only:: SOC_SPIRAM_SUPPORTED

        .. note::

            Apart from the crashed task's TCB and stack, data located in the external RAM will not be stored in the core dump file, this include variables defined with ``EXT_RAM_BSS_ATTR`` or ``EXT_RAM_NOINIT_ATTR`` attributes, as well as any data stored in the ``extram_bss`` section.

Core Dump to Flash
------------------

When the core dump file is saved to flash, the file is saved to a special core dump partition in flash. Specifying the core dump partition will reserve space on the flash chip to store the core dump file.

The core dump partition is automatically declared when using the default partition table provided by ESP-IDF. However, when using a custom partition table, you need to declare the core dump partition, as illustrated below:

.. code-block:: none

    # Name,   Type, SubType, Offset,  Size
    # Note: if you have increased the bootloader size, make sure to update the offsets to avoid overlap
    nvs,      data, nvs,     0x9000,  0x6000
    phy_init, data, phy,     0xf000,  0x1000
    factory,  app,  factory, 0x10000, 1M
    coredump, data, coredump,,        64K

.. important::

    If :doc:`../security/flash-encryption` is enabled on the device, please add an ``encrypted`` flag to the core dump partition declaration. Please note that the core dump cannot be read from encrypted partitions using ``idf.py coredump-info`` or ``idf.py coredump-debug`` commands.
    It is recommended to read the core dump from ESP which will automatically decrypt the partition and send it for analysis, which can be done by running e.g. ``idf.py coredump-info -c <path-to-core-dump>``.

    .. code-block:: none

        coredump, data, coredump,,       64K, encrypted

There are no special requirements for the partition name. It can be chosen according to the application's needs, but the partition type should be ``data`` and the sub-type should be ``coredump``. Also, when choosing partition size, note that the core dump file introduces a constant overhead of 20 bytes and a per-task overhead of 12 bytes. This overhead does not include the size of TCB and stack for every task. So the partition size should be at least ``20 + max tasks number x (12 + TCB size + max task stack size)`` bytes.

An example of the generic command to analyze core dump from flash is:

.. code-block:: bash

    idf.py coredump-info

or

.. code-block:: bash

    idf.py coredump-debug


.. note::

    The ``idf.py coredump-info`` and ``idf.py coredump-debug`` commands are wrappers around the `esp-coredump` tool for easier use in the ESP-IDF environment. For more information see :ref:`core_dump_commands` section.


Core Dump to UART
-----------------

When the core dump file is output to UART, the output file is Base64-encoded. The :ref:`CONFIG_ESP_COREDUMP_DECODE` option allows for selecting whether the output file is automatically decoded by the ESP-IDF monitor or kept encoded for manual decoding.


Automatic Decoding
^^^^^^^^^^^^^^^^^^

If :ref:`CONFIG_ESP_COREDUMP_DECODE` is set to automatically decode the UART core dump, ESP-IDF monitor will automatically decode the data, translate any function addresses to source code lines, and display it in the monitor. The output to ESP-IDF monitor would resemble the following output:

.. code-block:: none

    ===============================================================
    ==================== ESP32 CORE DUMP START ====================

    Crashed task handle: 0x3ffafba0, name: 'main', GDB name: 'process 1073413024'
    Crashed task is not in the interrupt context
    Panic reason: abort() was called at PC 0x400d66b9 on core 0

    ================== CURRENT THREAD REGISTERS ===================
    exccause       0x1d (StoreProhibitedCause)
    excvaddr       0x0
    epc1           0x40084013
    epc2           0x0
    ...
    ==================== CURRENT THREAD STACK =====================
    #0  0x4008110d in panic_abort (details=0x3ffb4f0b "abort() was called at PC 0x400d66b9 on core 0") at /builds/espressif/esp-idf/components/esp_system/panic.c:472
    #1  0x4008510c in esp_system_abort (details=0x3ffb4f0b "abort() was called at PC 0x400d66b9 on core 0") at /builds/espressif/esp-idf/components/esp_system/port/esp_system_chip.c:93
    ...
    ======================== THREADS INFO =========================
      Id   Target Id          Frame
    * 1    process 1073413024 0x4008110d in panic_abort (details=0x3ffb4f0b "abort() was called at PC 0x400d66b9 on core 0") at /builds/espressif/esp-idf/components/esp_system/panic.c:472
      2    process 1073413368 vPortTaskWrapper (pxCode=0x0, pvParameters=0x0) at /builds/espressif/esp-idf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:133
    ...
           TCB             NAME PRIO C/B  STACK USED/FREE
    ---------- ---------------- -------- ----------------
    0x3ffafba0             main      1/1         368/3724
    0x3ffafcf8            IDLE0      0/0         288/1240
    0x3ffafe50            IDLE1      0/0         416/1108
    ...
    ==================== THREAD 1 (TCB: 0x3ffafba0, name: 'main') =====================
    #0  0x4008110d in panic_abort (details=0x3ffb4f0b "abort() was called at PC 0x400d66b9 on core 0") at /builds/espressif/esp-idf/components/esp_system/panic.c:472
    #1  0x4008510c in esp_system_abort (details=0x3ffb4f0b "abort() was called at PC 0x400d66b9 on core 0") at /builds/espressif/esp-idf/components/esp_system/port/esp_system_chip.c:93
    ...
    ==================== THREAD 2 (TCB: 0x3ffafcf8, name: 'IDLE0') =====================
    #0  vPortTaskWrapper (pxCode=0x0, pvParameters=0x0) at /builds/espressif/esp-idf/components/freertos/FreeRTOS-Kernel/portable/xtensa/port.c:133
    #1  0x40000000 in ?? ()
    ...
    ======================= ALL MEMORY REGIONS ========================
    Name   Address   Size   Attrs
    ...
    .iram0.vectors 0x40080000 0x403 R XA
    .iram0.text 0x40080404 0xb8ab R XA
    .dram0.data 0x3ffb0000 0x2114 RW A
    ...
    ===================== ESP32 CORE DUMP END =====================
    ===============================================================

The :ref:`CONFIG_ESP_COREDUMP_UART_DELAY` allows for an optional delay to be added before the core dump file is output to UART.

Manual Decoding
^^^^^^^^^^^^^^^

If you set :ref:`CONFIG_ESP_COREDUMP_DECODE` to no decoding, then the raw  Base64-encoded body of core dump is output to UART between the following header and footer of the UART output:

.. code-block:: none

    ================= CORE DUMP START =================
    <body of Base64-encoded core dump, save it to file on disk>
    ================= CORE DUMP END ===================

It is advised to manually save the core dump text body to a file. The ``CORE DUMP START`` and ``CORE DUMP END`` lines must not be included in a core dump text file. The saved text can the be decoded using the following command:

.. code-block:: bash

    idf.py coredump-info -c </path/to/saved/base64/text>

or

.. code-block:: bash

    idf.py coredump-debug -c </path/to/saved/base64/text>


.. _core_dump_commands:

Core Dump Commands
------------------

ESP-IDF provides special commands to retrieve and analyze core dumps:

* ``idf.py coredump-info`` - reads coredump from flash and prints crashed task's registers, call stack, list of available tasks in the system, memory regions, and contents of memory stored in core dump (TCBs and stacks).
* ``idf.py coredump-debug`` - reads coredump from flash, saves it as ELF file and runs a GDB debug session with this file. You can examine memory, variables, and task states manually. Note that since not all memory is saved in the core dump, only the values of variables allocated on the stack are meaningful.

``idf.py coredump-info --help`` and ``idf.py coredump-debug --help`` commands can be used to get more details on usage. For example, they can save the coredump into a file and avoid the need to read it from flash every time these commands are run.

For advanced users who want to pass additional arguments or use custom ELF files, it is possible to use the `esp-coredump <https://github.com/espressif/esp-coredump>`_ tool directly. For more information, use in ESP-IDF environment:

.. code-block:: bash

    esp-coredump --help


ROM Functions in Backtraces
---------------------------

It is a possible that at the moment of a crash, some tasks and/or the crashed task itself have one or more ROM functions in their call stacks. Since ROM is not part of the program ELF, it is impossible for GDB to parse such call stacks due to GDB analyzing functions' prologues to decode backtraces. Thus, call stack parsing will break with an error message upon the first ROM function that is encountered.

To overcome this issue, the `ROM ELF <https://github.com/espressif/esp-rom-elfs/releases>`_ provided by Espressif is loaded automatically by ESP-IDF monitor based on the target and its revision. More details about ROM ELFs can be found in `esp-rom-elfs <https://github.com/espressif/esp-rom-elfs/blob/master/README.md>`_.


Dumping Variables on Demand
---------------------------

Sometimes you want to read the last value of a variable to understand the root cause of a crash. Core dump supports retrieving variable data over GDB by applying special attributes to declared variables.


Supported Notations and RAM Regions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list::

   * ``COREDUMP_DRAM_ATTR`` places the variable into the DRAM area, which is included in the dump.
   :SOC_RTC_FAST_MEM_SUPPORTED or SOC_RTC_SLOW_MEM_SUPPORTED: * ``COREDUMP_RTC_ATTR`` places the variable into the RTC area, which is included in the dump.
   :SOC_RTC_FAST_MEM_SUPPORTED: * ``COREDUMP_RTC_FAST_ATTR`` places the variable into the RTC_FAST area, which is included in the dump.


Example
^^^^^^^

1. In :ref:`project-configuration-menu`, enable :ref:`COREDUMP TO FLASH <CONFIG_ESP_COREDUMP_TO_FLASH_OR_UART>`, then save and exit.

2. In your project, create a global variable in the DRAM area, such as:

.. code-block:: bash

   // uint8_t global_var;
   COREDUMP_DRAM_ATTR uint8_t global_var;

3. In the main application, set the variable to any value and ``assert(0)`` to cause a crash.

.. code-block:: bash

   global_var = 25;
   assert(0);

4. Build, flash, and run the application on a target device and wait for the dumping information.

5. Run the command below to start core dumping in GDB, where ``PORT`` is the device USB port:

.. code-block:: bash

   idf.py coredump-debug

6. In GDB shell, type ``p global_var`` to get the variable content:

.. code-block:: bash

   (gdb) p global_var
   $1 = 25 '\031'

Related Documents
^^^^^^^^^^^^^^^^^

.. toctree::
    :maxdepth: 1

    core_dump_internals
````

## File: docs/en/api-guides/cplusplus.rst
````
C++ Support
===========

:link_to_translation:`zh_CN:[中文]`

.. highlight:: cpp

ESP-IDF is primarily written in C and provides C APIs. However, ESP-IDF supports development of applications in C++. This document covers various topics relevant to C++ development.

The following C++ features are supported:

- :ref:`cplusplus_exceptions`
- :ref:`cplusplus_multithreading`
- :ref:`cplusplus_rtti`
- :doc:`thread-local-storage` (``thread_local`` keyword)
- :ref:`cplusplus_filesystem`
- All C++ features implemented by GCC, except for some :ref:`cplusplus_limitations`. See `GCC documentation <https://gcc.gnu.org/projects/cxx-status.html>`_ for details on features implemented by GCC.


``esp-idf-cxx`` Component
-------------------------

`esp-idf-cxx <https://github.com/espressif/esp-idf-cxx>`_ component provides higher-level C++ APIs for some of the ESP-IDF features. This component is available from the `ESP Component Registry <https://components.espressif.com/components/espressif/esp-idf-cxx>`_.


C++ Language Standard
---------------------

By default, ESP-IDF compiles C++ code using C++26 language standard with GNU extensions (``-std=gnu++26``) for chip targets. For Linux targets, ESP-IDF selects the highest C+ standard supported by your host compiler. To use the highest C++ standard, upgrade your Linux toolchain to a version that supports it.

To compile the source code of a certain component using a different language standard, set the desired compiler flag in the component's ``CMakeLists.txt`` file:

.. code-block:: cmake

    idf_component_register( ... )
    target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)

Use ``PUBLIC`` instead of ``PRIVATE`` if the public header files of the component also need to be compiled with the same language standard.


.. _cplusplus_multithreading:

Multithreading
--------------

C++ threads, mutexes, and condition variables are supported. C++ threads are built on top of pthreads, which in turn wrap FreeRTOS tasks.

See :example:`cxx/pthread` for an example of creating threads in C++. Specifically, this example demonstrates how to use the ESP-pthread component to modify the stack sizes, priorities, names, and core affinities of C++ threads.

.. note::

    The destructor of `std::jthread <https://en.cppreference.com/w/cpp/thread/jthread>`_ can only safely be called from a task that has been created by :ref:`posix_thread_api` or by the `C++ threading library API <https://en.cppreference.com/w/cpp/thread>`_.


.. _cplusplus_exceptions:

Exception Handling
------------------

Support for C++ Exceptions in ESP-IDF is disabled by default, but can be enabled using the :ref:`CONFIG_COMPILER_CXX_EXCEPTIONS` option.

If an exception is thrown, but there is no ``catch`` block, the program is terminated by the ``abort`` function, and the backtrace is printed. See :doc:`fatal-errors` for more information about backtraces.

C++ Exceptions should **only** be used for exceptional cases, i.e., something happening unexpectedly and occurs rarely, such as events that happen less frequently than 1/100 times. **Do not** use them for control flow (see also the section about resource usage below). For more information on how to use C++ Exceptions, see the `ISO C++ FAQ <https://isocpp.org/wiki/faq/exceptions>`_ and `CPP Core Guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-errors>`_.

See :example:`cxx/exceptions` for an example of C++ exception handling. Specifically, this example demonstrates how to enable and use C++ exceptions in {IDF_TARGET_NAME}, with a class that throws an exception from the constructor if the provided argument is equal to 0.

C++ Exception Handling and Resource Usage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Enabling exception handling normally increases application binary size by a few KB.

Additionally, it may be necessary to reserve some amount of RAM for the exception emergency memory pool. Memory from this pool is used if it is not possible to allocate an exception object from the heap.

The amount of memory in the emergency pool can be set using the :ref:`CONFIG_COMPILER_CXX_EXCEPTIONS_EMG_POOL_SIZE` variable.

Some additional stack memory (around 200 bytes) is also used if and only if a C++ Exception is actually thrown, because it requires calling some functions from the top of the stack to initiate exception handling.

The run time of code using C++ exceptions depends on what actually happens at run time.

- If no exception is thrown, the code tends to be somewhat faster since there is no need to check error codes.
- If an exception is thrown, the run time of the code that handles exceptions is orders of magnitude slower than code returning an error code.

If an exception is thrown, the run time of the code that unwinds the stack is orders of magnitude slower than code returning an error code. The significance of the increased run time will depend on the application's requirements and implementation of error handling (e.g., requiring user input or messaging to a cloud). As a result, exception-throwing code should never be used in real-time critical code paths.


.. _cplusplus_rtti:

Runtime Type Information (RTTI)
-------------------------------

Support for RTTI in ESP-IDF is disabled by default, but can be enabled using :ref:`CONFIG_COMPILER_CXX_RTTI` option.

Enabling this option compiles all C++ files with RTTI support enabled, which allows using ``dynamic_cast`` conversion and ``typeid`` operator. Enabling this option typically increases the binary size by tens of kB.

See :example:`cxx/rtti` for an example of using RTTI in ESP-IDF. Specifically, this example demonstrates how to use the RTTI feature in ESP-IDF, enabling compile time support for RTTI, and showing how to print demangled type names of objects and functions, and how dynamic_cast behaves with objects of two classes derived from a common base class.

.. _cplusplus_filesystem:

Filesystem Library
------------------

C++ Filesystem library (``#include <filesystem>``) features are supported in ESP-IDF, with the following exceptions:

- Since symbolic and hard links are not supported in ESP-IDF, related functions are not implemented.
- ``std::filesystem::space`` is not implemented.
- ``std::filesystem::resize_file`` is not implemented.
- ``std::filesystem::current_path`` always returns ``/``. Setting the current path is not supported.
- ``std::filesystem::permissions`` doesn't support setting file permissions.

Note that the choice of the filesystem also affects the behavior of the filesystem library. For example, SPIFFS filesystem has limited support for directories, therefore the related std::filesystem functions may not work as they do on a filesystem which does support directories.

Developing in C++
-----------------

The following sections provide tips on developing ESP-IDF applications in C++.


Combining C and C++ Code
^^^^^^^^^^^^^^^^^^^^^^^^

When an application is developed using both C and C++, it is important to understand the concept of `language linkage <https://en.cppreference.com/w/cpp/language/language_linkage>`_.

In order for a C++ function to be callable from C code, it has to be both **declared** and **defined** with C linkage (``extern "C"``):

.. code-block:: cpp

    // declaration in the .h file:
    #ifdef __cplusplus
    extern "C" {
    #endif

    void my_cpp_func(void);

    #ifdef __cplusplus
    }
    #endif

    // definition in a .cpp file:
    extern "C" void my_cpp_func(void) {
        // ...
    }


In order for a C function to be callable from C++, it has to be **declared** with C linkage:

.. code-block:: c

    // declaration in .h file:
    #ifdef __cplusplus
    extern "C" {
    #endif

    void my_c_func(void);

    #ifdef __cplusplus
    }
    #endif

    // definition in a .c file:
    void my_c_func(void) {
        // ...
    }


Defining ``app_main`` in C++
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP-IDF expects the application entry point, ``app_main``, to be defined with C linkage. When ``app_main`` is defined in a .cpp source file, it has to be designated as ``extern "C"``:

.. code-block:: cpp

    extern "C" void app_main()
    {
    }


.. _cplusplus_designated_initializers:

Designated Initializers
^^^^^^^^^^^^^^^^^^^^^^^

Many of the ESP-IDF components use :ref:`api_reference_config_structures` as arguments to the initialization functions. ESP-IDF examples written in C routinely use `designated initializers <https://en.cppreference.com/w/c/language/struct_initialization>`_ to fill these structures in a readable and a maintainable way.

C and C++ languages have different rules with regards to the designated initializers. For example, C++26 (currently the default in ESP-IDF) does not support out-of-order designated initialization, nested designated initialization, mixing of designated initializers and regular initializers, and designated initialization of arrays. Therefore, when porting ESP-IDF C examples to C++, some changes to the structure initializers may be necessary. See the `C++ aggregate initialization reference <https://en.cppreference.com/w/cpp/language/aggregate_initialization>`_ for more details.


``iostream``
^^^^^^^^^^^^

``iostream`` functionality is supported in ESP-IDF, with a couple of caveats:

1. Normally, ESP-IDF build process eliminates the unused code. However, in the case of iostreams, simply including ``<iostream>`` header in one of the source files significantly increases the binary size by about 200 kB.
2. By default, ESP-IDF uses a simple non-blocking implementation of the standard input stream (``stdin``). To get the usual behavior of ``std::cin``, the application has to initialize the UART driver and enable the blocking mode as shown in :example_file:`common_components/protocol_examples_common/stdin_out.c`.


.. _cplusplus_limitations:

Limitations
-----------

- Linker script generator does not support function level placements for functions with C++ linkage.
- Vtables are placed into Flash and are not accessible when the flash cache is disabled. Therefore, virtual function calls should be avoided in :ref:`iram-safe-interrupt-handlers`. Placement of Vtables cannot be adjusted using the linker script generator, yet.


What to Avoid
-------------

Do not use ``setjmp``/``longjmp`` in C++. ``longjmp`` blindly jumps up the stack without calling any destructors, easily introducing undefined behavior and memory leaks. Use C++ exceptions instead, they guarantee correctly calling destructors. If you cannot use C++ exceptions, use alternatives (except ``setjmp``/``longjmp`` themselves) such as simple return codes.
````

## File: docs/en/api-guides/current-consumption-measurement-modules.rst
````
Current Consumption Measurement of Modules
==========================================

:link_to_translation:`zh_CN:[中文]`

{IDF_TARGET_SOC_BOOT_PIN:default="Not updated", esp32="IO0", esp32s2="IO0", esp32s3="IO0", esp32c3="IO9", esp32c2="IO9", "esp32c6"="IO9", "esp32h2"="IO9", "esp32p4"="IO35", "esp32c5"="IO28"}

You may want to know the current consumption of a `module <https://www.espressif.com/en/products/modules>`__ in Deep-sleep mode, :doc:`other power-saving modes </api-reference/system/sleep_modes>`, and Active mode to develop some applications sensitive to power consumption. This section introduces how to measure the current consumption of a module running such an application.


Notes to Measurement
--------------------

Can We Use a Development Board?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: esp32c6

    For {IDF_TARGET_NAME}, you can use development boards such as `ESP32-C6-DevKitC-1 <https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/index.html>`__ and `ESP32-C6-DevKitM-1 <https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitm-1/index.html>`__ to measure current consumption of corresponding modules as these development boards are equipped with headers, which can be used to measure current drawn by modules.

.. only:: esp32h2

    For {IDF_TARGET_NAME}, you can use development boards such as `ESP32-H2-DevKitM-1 <https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32h2/esp32-h2-devkitm-1/index.html>`__ to measure current consumption of corresponding modules as these development boards are equipped with headers, which can be used to measure current drawn by modules.

.. only:: esp32c5

    For {IDF_TARGET_NAME}, you can use development boards such as `ESP32-C5-DevKitC-1 <https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c5/esp32-c5-devkitc-1/index.html>`__ to measure current consumption of corresponding modules as these development boards are equipped with headers, which can be used to measure current drawn by modules.

.. only:: esp32c6 or esp32h2 or esp32c5

    With such development boards, you can measure current consumption of modules in Deep-sleep mode by flashing chips with the :example:`deep_sleep <system/deep_sleep>` example. However, you can also measure current of bare modules equipped with {IDF_TARGET_NAME} chip using the following method.

.. only:: esp32 or esp32s2 or esp32s3 or esp32c2 or esp32c3 or esp32p4

    For {IDF_TARGET_NAME}, using a development board directly to measure current consumption of the corresponding module is not recommended, as some circuits still consume power on the board even when you flash the chip with the :example:`deep_sleep <system/deep_sleep>` example. Therefore, you need to cut off the power supply circuit to the module to measure the module's current. This method is inconvenient and increases measurement costs.


How to Choose an Appropriate Ammeter?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the :example:`deep_sleep <system/deep_sleep>` example, the module will be woken up every 20 seconds. In Deep-sleep mode, the current in the module is just several microamps (μA), while in active mode, the current is in the order of milliamps (mA). The high dynamic current range makes accurate measurement difficult. Ordinary ammeters cannot dynamically switch the measurement range fast enough.

Additionally, ordinary ammeters have a relatively high internal resistance, resulting in a significant voltage drop. This may cause the module to enter an unstable state, as it is powered by a voltage smaller than the minimum required voltage supply.

Therefore, an ammeter suitable for measuring current in Deep-sleep mode should have low internal resistance and, ideally, switch current ranges dynamically. We recommend two options: the `Joulescope ammeter <https://www.joulescope.com/>`__ and the `Power Profiler Kit II from Nordic <https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2.?lang=en>`__.


Joulescope Ammeter
""""""""""""""""""

The Joulescope ammeter combines high-speed sampling and rapid dynamic current range switching to provide accurate and seamless current and energy measurements, even for devices with rapidly varying current consumption. Joulescope accurately measures electrical current over nine orders of magnitude from amps down to nanoamps. This wide range allows for accurate and precise current measurements for devices. Additionally, Joulescope has a total voltage drop of 25 mV at 1 A, which keeps the module running normally. These two features make Joulescope a perfect option for measuring the module switching between Deep-sleep mode and wake-up mode.

Joulescope has no display screen. You need to connect it to a PC to visualize the current waveforms of the measured module. For specific instructions, please follow the documentation provided by the manufacturer.


Nordic Power Profiler Kit II
""""""""""""""""""""""""""""

The Nordic Power Profiler Kit II has an advanced analog measurement unit with a high dynamic measurement range. This allows for accurate power consumption measurements for the entire range typically seen in low-power embedded applications, all the way from several microamps to 1 A. The resolution varies between 100 nA and 1 mA, depending on the measurement range, and is high enough to detect small spikes often seen in low-power optimized systems.


Hardware Connection
-------------------

To measure the power consumption of a bare module, you need an `ESP-Prog <https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/other/esp-prog/user_guide.html>`__ to flash the :example:`deep_sleep <system/deep_sleep>` example to the module and power the module during measurement, a suitable ammeter (here we use the Joulescope ammeter), a computer, and of course a bare module with necessary jumper wires. For the connection, please refer to the following figure.

.. figure:: /../_static/hardware_connection_power_measure.png
    :align: center
    :scale: 80%
    :alt: Hardware Connection (click to enlarge)

    Hardware Connection (click to enlarge)

Please connect the pins of **UART TX**, **UART RX**, **SPI Boot**, **Enable**, and **Ground** on the measured module with corresponding pins on ESP-Prog, and connect the **VPROG** pin on ESP-Prog with the **IN+** port on the Joulescope ammeter and connect its **OUT+** port with the **Power supply (3V3)** pin on the measured module. For the specific names of these pins in different modules, please refer to the list below.

.. list-table:: Pin Names of Modules Based on {IDF_TARGET_NAME} Chip
    :header-rows: 1
    :widths: 50 50
    :align: center

    * - Function of Module Pin
      - Pin Name

    * - UART TX
      - TXD0

    * - UART RX
      - RXD0

    * - SPI Boot
      - {IDF_TARGET_SOC_BOOT_PIN}

    * - Enable
      - EN

    * - Power Supply
      - 3V3

    * - Ground
      - GND

.. only:: esp32

    For modules based on the ESP32 chip, the names of UART TX and UART RX pins may also be U0TXD and U0RXD.

.. only:: esp32c2

    For modules based on the ESP32-C2 chip, the names of UART TX and UART RX pins may also be TXD and RXD.

.. only:: esp32c3

    For modules based on the ESP32-C3 chip, the names of UART TX and UART RX pins may also be TXD and RXD, TX and RX, or TX0 and RX0.

For details of the pin names, please refer to the `datasheet of specific module <https://www.espressif.com/en/support/download/documents/modules>`__.


Measurement Steps
-----------------

ESP32-S3-WROOM-1 is used as an example in the measurement, and other modules can be measured similarly. For the specific current consumption of chips in different modes, please refer to the Current Consumption subsection in the corresponding `chip datasheet <https://www.espressif.com/en/support/download/documents/chips?keys=&field_download_document_type_tid%5B%5D=510>`__.

You can refer to the following steps to measure the current in Deep-sleep mode.

- Connect the aforementioned devices according to the hardware connection.

- Flash the :example:`deep_sleep <system/deep_sleep>` example to the module. For details, please refer to :doc:`Start a Project on Linux and macOS </get-started/linux-macos-start-project>` for a computer with Linux or macOS system or :doc:`Start a Project on Windows </get-started/windows-start-project>` for a computer with Windows system.

.. only:: esp32 or esp32s2 or esp32s3

    Please note that when you configure the example by running ``idf.py menuconfig``, you need to disable ``Enable touch wake up`` in the ``Example Configuration`` to reduce the bottom current.

.. only:: esp32

    For modules with an external resistor on GPIO12 (such as ESP32-WROVER-E/IE), you should call :cpp:func:`rtc_gpio_isolate` before going into Deep-sleep. This is to isolate the GPIO12 pin from external circuits to further minimize current consumption. Please note, for other modules, you do not have to call this function, otherwise, you may get abnormal results.

- By default, the module will be woken up every 20 seconds (you can change the timing by modifying the code of this example). To check if the example runs as expected, you can monitor the module operation by running ``idf.py -p PORT monitor`` (please replace PORT with your serial port name).

- Open the Joulescope software to see the current waveform as shown in the image below.

From the waveforms, you can obtain that the current of the module in Deep-sleep mode is 8.14 μA. In addition, you can also see the current of the module in active mode, which is about 23.88 mA. The waveforms also show that the average power consumption during Deep-sleep mode is 26.85 μW, and the average power consumption during active mode is 78.32 mW.

.. figure:: /../_static/current_measure_waveform.png
    :align: center
    :scale: 100%
    :alt: Current Waveform of ESP32-S3-WROOM-1 (click to enlarge)

    Current Waveform of ESP32-S3-WROOM-1 (click to enlarge)

The figure below shows the total power consumption of one cycle is 6.37 mW.

.. figure:: /../_static/power_measure_waveform.png
    :align: center
    :scale: 100%
    :alt: Power Consumption of ESP32-S3-WROOM-1 (click to enlarge)

    Power Consumption of ESP32-S3-WROOM-1 (click to enlarge)

By referring to these power consumption in different modes, you can estimate the power consumption of your applications and choose the appropriate power source.
````

## File: docs/en/api-guides/deep-sleep-stub.rst
````
Deep-sleep Wake Stubs
=====================

:link_to_translation:`zh_CN:[中文]`

Introduction
------------

The wakeup time from Deep-sleep mode is much longer, compared to Light-sleep and Modem-sleep modes as ROM and RAM are both powered down in this case, and the CPU needs more time for SPI booting. However, {IDF_TARGET_NAME} supports running a “Deep-sleep wake stub” when coming out of Deep-sleep. This function runs immediately as soon as the chip wakes up - before any normal initialization, bootloader, or ESP-IDF code has run.

Specifically, after waking up from Deep-sleep mode, {IDF_TARGET_NAME} starts partial initialization. Then RTC fast memory will be validated with CRC. If validation passes, the wake stub code will be executed.

As {IDF_TARGET_NAME} has just woken up from Deep-sleep, most of the peripherals are in the reset state. The SPI flash has not been mapped. Thus, wake stub code can only call functions implemented in ROM or loaded into RTC fast memory, which retains content during Deep-sleep.

From the above, by utilizing the wake stub functionality in an application, you can quickly run some code when waking up from Deep-sleep mode, without having to wait for the whole boot-up process. However, the stub size is restricted by the size of RTC fast memory.

.. only:: SOC_RTC_SLOW_MEM_SUPPORTED

    {IDF_TARGET_NAME} supports RTC memory, including both RTC fast memory and RTC slow memory. The wake stub code should be loaded into RTC fast memory, with data utilized by the code being stored in RTC fast or RTC slow memory.

.. only:: not SOC_RTC_SLOW_MEM_SUPPORTED

    {IDF_TARGET_NAME} only supports RTC fast memory. The wake stub code and data that it utilizes should be loaded into RTC fast memory.

Next we will introduce how to implement the wake stub code in an application.

Implement wake stub
-------------------

The wake stub in esp-idf is realized by the function :cpp:func:`esp_wake_deep_sleep()`. This function is executed whenever the SoC wakes from Deep-sleep. As this function is weakly-linked to the default function :cpp:func:`esp_default_wake_deep_sleep()`, if your application contains a function with the name ``esp_wake_deep_sleep()``, the default version :cpp:func:`esp_default_wake_deep_sleep()` in esp-idf will be overridden.

Please note that implementing the function :cpp:func:`esp_wake_deep_sleep()` in your application is not mandatory for utilizing the Deep-sleep functionality. It becomes necessary only if you want to introduce certain behavior immediately upon the SoC's wake-up.

When you develop a customized wake stub, the first step it should do is to call the default function :cpp:func:`esp_default_wake_deep_sleep()`.

In addition, you can switch between different wake stubs by calling the function :cpp:func:`esp_set_deep_sleep_wake_stub()` during runtime.

Implementing the wake stub function in your application includes the following steps:

.. list::

    - Loading wake stub code into RTC fast memory
    :SOC_RTC_SLOW_MEM_SUPPORTED: - Loading data into RTC memory
    :not SOC_RTC_SLOW_MEM_SUPPORTED: - Loading data into RTC fast memory

Load Wake Stub Code into RTC Fast Memory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The wake stub code can only call functions present in ROM or loaded into RTC fast memory. All other RAM locations are unintiailized and contain random data. While the wake stub code can use other RAM areas for temporary storage, the contents of these areas will be overwritten either upon returning to Deep-sleep mode or upon initiating esp-idf.

Wake stub code is a part of the main esp-idf application. During regular execution of esp-idf, functions can call the wake stub code or access RTC memory, treating them as a regular part of the application.

Wake stub code must reside in RTC fast memory. This can be realized in two ways.

- Employ the attribute ``RTC_IRAM_ATTR`` to place :cpp:func:`esp_wake_deep_sleep()` into RTC fast memory:

.. code:: c

    void RTC_IRAM_ATTR esp_wake_deep_sleep(void) {
        esp_default_wake_deep_sleep();
        // Add additional functionality here
    }

The first approach is suitable for short and simple code segments or for source files including both "normal" and "RTC" code.

- Place the function :cpp:func:`esp_wake_deep_sleep()` into any source file with name starting with ``rtc_wake_stub``. For files with such names ``rtc_wake_stub*``, their contents can be automatically put into RTC fast memory by the linker.

The second method is preferable when writing longer code segments in RTC fast memory.

.. only:: SOC_RTC_SLOW_MEM_SUPPORTED

    Load Wake Stub Data into RTC memory
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    RTC memory must include read-only data used by the wake stub code. Data in RTC memory is initialized whenever the SoC restarts, except when waking from Deep-sleep. In such cases, the data retained before entering to Deep-sleep are kept. Data used by the wake stub code must be resident in RTC memory, i.e. RTC fast memory or in RTC slow memory.

    The data can be specified in the following two methods:

    - Utilize attributes ``RTC_DATA_ATTR`` and ``RTC_RODATA_ATTR`` to specify writable or read-only data, respectively.

    .. code:: c

        RTC_DATA_ATTR int wake_count;

        void RTC_IRAM_ATTR esp_wake_deep_sleep(void) {
            esp_default_wake_deep_sleep();
            static RTC_RODATA_ATTR const char fmt_str[] = "Wake count %d\n";
            esp_rom_printf(fmt_str, wake_count++);
        }

    The RTC memory area where the data will be placed can be configured via the menuconfig option :ref:`CONFIG_{IDF_TARGET_CFG_PREFIX}_RTCDATA_IN_FAST_MEM`. This option allows keeping slow memory area for ULP programs. Once it is enabled, the data marked with ``RTC_DATA_ATTR`` and ``RTC_RODATA_ATTR`` are placed in the RTC fast memory segment; otherwise, it goes to RTC slow memory (the default option). This option depends on the :ref:`CONFIG_FREERTOS_UNICORE` option because RTC fast memory can be accessed only by PRO_CPU.

    .. only:: esp32

        This option depends on the :ref:`CONFIG_FREERTOS_UNICORE` because RTC fast memory can be accessed only by PRO_CPU.

        The attributes ``RTC_FAST_ATTR`` and ``RTC_SLOW_ATTR`` can be used to specify data that is forcefully placed into RTC fast memory and RTC slow memory, respectively. Any access to data marked with ``RTC_FAST_ATTR`` is allowed by PRO_CPU only.

    .. only:: esp32s2 or esp32s3

        The attributes ``RTC_FAST_ATTR`` and ``RTC_SLOW_ATTR`` can be used to specify data that is forcefully placed into RTC fast memory and RTC slow memory, respectively.


.. only:: not SOC_RTC_SLOW_MEM_SUPPORTED

    Load Wake Stub Data into RTC Fast memory
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Data used by wake stub code must be resident in RTC fast memory.

    The data can be specified in the following two methods:

    - Use the ``RTC_DATA_ATTR`` and ``RTC_RODATA_ATTR`` to specify writable or read-only data, respectively.

    .. code:: c

        RTC_DATA_ATTR int wake_count;

        void RTC_IRAM_ATTR esp_wake_deep_sleep(void) {
            esp_default_wake_deep_sleep();
            static RTC_RODATA_ATTR const char fmt_str[] = "Wake count %d\n";
            esp_rom_printf(fmt_str, wake_count++);
        }

    The attributes ``RTC_FAST_ATTR`` and ``RTC_SLOW_ATTR`` can be used to specify data that will be force placed into RTC fast memory and RTC slow memory respectively. However, {IDF_TARGET_NAME} includes RTC fast memory only, so both these two attributes will map to this region.

However, any string constants used in this way must be declared as arrays and marked with ``RTC_RODATA_ATTR``, as shown in the example above.

- Place the data into any source file with name starting with ``rtc_wake_stub``, as demonstrated in the example source file :example_file:`system/deep_sleep_wake_stub/main/rtc_wake_stub_example.c`.

.. code:: c

    if (s_count >= s_max_count) {
        // Reset s_count
        s_count = 0;

        // Set the default wake stub.
        // There is a default version of this function provided in esp-idf.
        esp_default_wake_deep_sleep();

        // Return from the wake stub function to continue
        // booting the firmware.
        return;
    }

The second approach is advisable when incorporating strings or more complex code segments.

You can enable the Kconfig option :ref:`CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP` to reduce wake-up time. See more information in :doc:`Fast boot from Deep-sleep <bootloader>`.

All of the above functions are declared in :component_file:`esp_hw_support/include/esp_sleep.h`.

Application Examples
--------------------

.. only:: SOC_RTC_FAST_MEM_SUPPORTED

    - :example:`system/deep_sleep_wake_stub` demonstrates how to use the Deep-sleep wake stub on {IDF_TARGET_NAME} to quickly perform some tasks (the wake stub code) immediately after wake-up before going back to sleep.

Measure Time from Deep-sleep Wake-up to Wake Stub Execution
-------------------------------------------------------------

In certain low-power scenarios, you may want to measure the time it takes for an {IDF_TARGET_NAME} chip to wake up from Deep-sleep to executing the wake stub function.

This section describes two methods for measuring this wake-up duration.

Method 1: Estimate Using CPU Cycle Count
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This method uses the CPU's internal cycle counter to estimate the wake-up time. At the beginning of the stub (with the function type of `esp_deep_sleep_wake_stub_fn_t`), the current CPU cycle count is read and converted into time based on the running CPU frequency.

Reference example: :example:`system/deep_sleep_wake_stub`.

After running the example, you will see a log similar to:

.. code-block:: bash

    Enabling timer wakeup, 10s
    Entering deep sleep
    ESP-ROM:esp32c3-api1-20210207
    Build:Feb  7 2021
    rst:0x5 (DSLEEP),boot:0xc (SPI_FAST_FLASH_BOOT)
    wake stub: wakeup count is 1, wakeup cause is 8, wakeup cost 12734 us
    wake stub: going to deep sleep
    ESP-ROM:esp32c3-api1-20210207
    Build:Feb  7 2021
    rst:0x5 (DSLEEP),boot:0xc (SPI_FAST_FLASH_BOOT)

The ``wakeup cost 12734 us`` is time between Deep-sleep wake-up and wake stub execution.

Advantages:

- Requires no external hardware.
- Easy to implement.

Limitations:

- The measured duration may include part of the initialization flow.
- Not suitable for ultra-precise timing analysis.

Method 2: Use GPIO pins and Logic Analyzer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can use one GPIO pin as the wake-up source and another GPIO pin to indicate when the wake stub begins execution. By observing the timing between these GPIO transitions on a logic analyzer, you can obtain an accurate measurement of the time from wake-up to stub execution.

For example, in the screenshot below, GPIO4 functions as the wake-up source, and GPIO5 indicates when the wake stub begins execution. The timing between the high level of GPIO4 and GPIO5 is the time from wake-up to stub execution.

.. figure:: ../../_static/deep-sleep-stub-logic-analyzer-result.png
    :align: center
    :alt: Time from Wake-up to Stub Execution
    :width: 100%

    Time from Wake-up to Stub Execution

The ``2.657ms`` is time between Deep-sleep wake-up and wake stub execution.

Advantages:

- High accuracy.
- Useful for validating hardware timing behavior.

Limitations:

- Requires external equipment (logic analyzer or oscilloscope).
- May require test pin wiring on custom boards.

Recommendation
^^^^^^^^^^^^^^^^

- For quick estimation or software-only testing, Method 1 is sufficient.
- For precise validation and hardware-level timing, Method 2 is recommended.
````

## File: docs/en/api-guides/dfu.rst
````
Device Firmware Upgrade via USB
===============================

:link_to_translation:`zh_CN:[中文]`

.. only:: not SOC_USB_SERIAL_JTAG_SUPPORTED

    Typically, the firmware of {IDF_TARGET_NAME} is flashed via the chip's serial port. However, flashing via the serial port requires a USB to serial converter chip (e.g., CP210x or FTDI) to be connected to {IDF_TARGET_NAME}. Please see :doc:`Establish Serial Connection with {IDF_TARGET_NAME} <../get-started/establish-serial-connection>` for more details. {IDF_TARGET_NAME} contains a USB OTG peripheral, making it possible to connect {IDF_TARGET_NAME} to the host directly via USB (thus not requiring a USB to serial converter chip).

.. only:: SOC_USB_SERIAL_JTAG_SUPPORTED

    Typically, the firmware of {IDF_TARGET_NAME} is flashed via the chip's serial port or USB_SERIAL_JTAG (see :doc:`Establish Serial Connection with {IDF_TARGET_NAME} <../get-started/establish-serial-connection>` for more details). {IDF_TARGET_NAME} also contains a USB OTG peripheral making it possible to connect {IDF_TARGET_NAME} to the host directly via USB Device Firmware Upgrade.

.. only:: esp32s3

    By default, the :doc:`USB_SERIAL_JTAG <usb-serial-jtag-console>` module is connected to {IDF_TARGET_NAME}'s internal USB PHY, while the USB OTG peripheral can be used only if an external USB PHY is connected. Since DFU is provided via the USB OTG peripheral, it cannot be used through the internal PHY in this configuration.

    However, you can permanently switch the internal USB PHY to work with USB OTG peripheral instead of USB_SERIAL_JTAG by burning the ``USB_PHY_SEL`` eFuse. See *{IDF_TARGET_NAME} Technical Reference Manual* [`PDF <{IDF_TARGET_TRM_EN_URL}>`__] for more details about USB_SERIAL_JTAG and USB OTG.

Device Firmware Upgrade (DFU) is a mechanism for upgrading the firmware of {IDF_TARGET_NAME} directly via the Universal Serial Bus (USB). However, enabling Secure Boot or flash encryption disables the USB-OTG USB stack in the ROM, disallowing updates via the serial emulation or DFU on that port.

- :ref:`get-started-get-prerequisites` of the Getting Started Guide introduces the software requirements of DFU.
- Section :ref:`api_guide_dfu_build` describes how to build firmware for DFU with ESP-IDF.
- Section :ref:`api_guide_dfu_flash` deals with flashing the firmware.


USB Connection
--------------

.. only:: esp32p4

    {IDF_TARGET_NAME} routes the USB D+ and D- signals to their dedicated pins. For USB device functionality, these pins must be connected to the USB bus (e.g., via a Micro-B port, USB-C port, or directly to standard-A plug).

.. only:: esp32s2 or esp32s3

    The necessary connections for {IDF_TARGET_NAME}'s internal USB PHY (transceiver) are shown in the following table:

    .. list-table::
       :header-rows: 1
       :widths: 25 20

       * - GPIO
         - USB

       * - 20
         - D+ (green)

       * - 19
         - D- (white)

       * - GND
         - GND (black)

       * - +5V
         - +5V (red)

.. warning::

    Some cables are wired up with non-standard colors and some drivers are able to work with swapped D+ and D- connections. Please try to swap the cables connecting to D+ and D- if your device is not detected.

.. note::

    {IDF_TARGET_NAME} chip needs to be in bootloader mode before it can be detected as a DFU device and flash. Please refer to `Boot Mode Selection <https://docs.espressif.com/projects/esptool/en/latest/{IDF_TARGET_PATH_NAME}/advanced-topics/boot-mode-selection.html#select-bootloader-mode>`_ for more information about how to enter bootloader mode.


.. _api_guide_dfu_build:

Building the DFU Image
----------------------

The command below will create a DFU image named ``dfu.bin`` that is placed in the project's ``build`` directory::

    idf.py dfu

.. note::

    Do not forget to set the target chip by ``idf.py set-target`` before running ``idf.py dfu``. Otherwise, you might create an image for a different chip or receive an error message like ``unknown target 'dfu'``.


.. _api_guide_dfu_flash:

Flashing the DFU Image
----------------------

The command below will download the DFU image into {IDF_TARGET_NAME}::

    idf.py dfu-flash

The command relies on `dfu-util <http://dfu-util.sourceforge.net/>`_. Please see :ref:`get-started-get-prerequisites` for installing ``dfu-util``. ``dfu-util`` needs additional setup for :ref:`api_guide_dfu_flash_win` or setting up an :ref:`api_guide_dfu_flash_udev`. macOS users should be able to use ``dfu-util`` without further setup.

If there are more boards with the same chip connected then ``idf.py dfu-list`` can be used to list the available devices, for example::

    Found Runtime: [303a:0002] ver=0723, devnum=4, cfg=1, intf=2, path="1-10", alt=0, name="UNKNOWN", serial="0"
    Found Runtime: [303a:0002] ver=0723, devnum=6, cfg=1, intf=2, path="1-2", alt=0, name="UNKNOWN", serial="0"

Consequently, the desired device can be selected for flashing by the ``--path`` argument. For example, the devices listed above can be flashed individually by the following commands::

    idf.py dfu-flash --path 1-10
    idf.py dfu-flash --path 1-2

.. note::

    The vendor and product identificators are set based on the selected chip target by the ``idf.py set-target`` command and they are not selectable during the ``idf.py dfu-flash`` call.

See :ref:`api_guide_dfu_flash_errors` and their solutions.


.. _api_guide_dfu_flash_udev:

Udev Rule (Linux Only)
----------------------

Udev is a device manager for the Linux kernel. It allows running ``dfu-util`` (and ``idf.py dfu-flash``) without ``sudo`` for gaining access to the chip.

Create file ``/etc/udev/rules.d/40-dfuse.rules`` with the following content::

    SUBSYSTEMS=="usb", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="00??", GROUP="plugdev", MODE="0666"

.. note::

    Please check the output of the command ``groups``. You need to be a member of the `GROUP` specified above. You may use some other existing groups for this purpose (e.g., `uucp` on some systems instead of `plugdev`) or create a new group for this purpose.

Restart your computer so the previous setting could take into affect or run ``sudo udevadm trigger`` to force manually udev to trigger your new rule.


.. _api_guide_dfu_flash_win:

USB Drivers (Windows Only)
--------------------------

``dfu-util`` uses `libusb` to access the device. On Windows, the `WinUSB` driver is the recommended driver which has to be installed for the device to work properly. For more details please see the `libusb wiki <https://github.com/libusb/libusb/wiki/Windows#How_to_use_libusb_on_Windows>`_.

.. only:: esp32s2

    The development board driver can be downloaded from https://github.com/espressif/esp-win-usb-drivers/releases. The files need to be extracted and `installed <https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/using-an-inf-file-to-install-a-file-system-filter-driver#right-click-install>`_. This should change or install the WinUSB driver for the right interface of the device.

.. note::

    If the feature is not working please proceed with the manual driver assignment. Otherwise, the following section can be skipped.

USB Drivers (Windows Only) - manual driver assignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Manual driver assignment can be performed with the `Zadig tool <https://zadig.akeo.ie/>`_. Please make sure that the device is in download mode before running the tool and that the {IDF_TARGET_NAME} device is detected before the driver installation.

The Zadig tool might detect several USB interfaces of {IDF_TARGET_NAME}. Please install the `WinUSB` driver **only** for the interface where there is no driver installed (probably it is Interface 2) and do not re-install the driver for the other interface.

.. warning::

    The manual installation of the driver in Device Manager of Windows is not recommended because the flashing might not work properly.


.. _api_guide_dfu_flash_errors:

Common Errors and Known Issues
------------------------------

- ``dfu-util: command not found`` might indicate that the tool has not been installed or is not available from the terminal. An easy way of checking the tool is running ``dfu-util --version``. Please see :ref:`get-started-get-prerequisites` for installing ``dfu-util``.

- The reason for ``No DFU capable USB device available`` could be that the USB driver was not properly installed on Windows (see :ref:`api_guide_dfu_flash_win`), udev rule was not setup on Linux (see :ref:`api_guide_dfu_flash_udev`) or the device is not in bootloader mode.

- Flashing with ``dfu-util`` on Windows fails on the first attempt with error ``Lost device after RESET?``. Please retry the flashing and it should succeed the next time.


.. only:: SOC_SUPPORTS_SECURE_DL_MODE

    Secure Download Mode
    --------------------

    When Secure Download Mode is enabled, DFU is no longer possible. Please see :doc:`Flash Encryption <../security/flash-encryption>` guide for more details.
````

## File: docs/en/api-guides/error-handling.rst
````
.. highlight:: c

Error Handling
==============

:link_to_translation:`zh_CN:[中文]`

Overview
--------

Identifying and handling run-time errors is important for developing robust applications. There can be multiple kinds of run-time errors:

- Recoverable errors:

  - Errors indicated by functions through return values (error codes)
  - C++ exceptions, thrown using ``throw`` keyword

- Unrecoverable (fatal) errors:

  - Failed assertions (using ``assert`` macro and equivalent methods, see :ref:`assertions`) and ``abort()`` calls.
  - CPU exceptions: access to protected regions of memory, illegal instruction, etc.
  - System level checks: watchdog timeout, cache access error, stack overflow, stack smashing, heap corruption, etc.

This guide primarily introduces the error handling mechanisms in ESP-IDF for **recoverable errors** and provides common error handling patterns. Some sections also mention macros used for **unrecoverable errors**, with the aim of illustrating their use in scenarios with different levels of error severity.

For instructions on diagnosing unrecoverable errors, see :doc:`Fatal Errors <fatal-errors>`.


Error Codes
-----------

The majority of ESP-IDF-specific functions use :cpp:type:`esp_err_t` type to return error codes. :cpp:type:`esp_err_t` is a signed integer type. Success (no error) is indicated with ``ESP_OK`` code, which is defined as zero.

Various ESP-IDF header files define possible error codes using preprocessor defines. Usually these defines start with ``ESP_ERR_`` prefix. Common error codes for generic failures (out of memory, timeout, invalid argument, etc.) are defined in ``esp_err.h`` file. Various components in ESP-IDF may define additional error codes for specific situations.

For the complete list of error codes, see :doc:`Error Code Reference <../api-reference/error-codes>`.


Converting Error Codes to Error Messages
----------------------------------------

For each error code defined in ESP-IDF components, :cpp:type:`esp_err_t` value can be converted to an error code name using :cpp:func:`esp_err_to_name` or :cpp:func:`esp_err_to_name_r` functions. For example, passing ``0x101`` to :cpp:func:`esp_err_to_name` will return a ``ESP_ERR_NO_MEM`` string. Such strings can be used in log output to make it easier to understand which error has happened.

Additionally, :cpp:func:`esp_err_to_name_r` function will attempt to interpret the error code as a `standard POSIX error code <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html>`_, if no matching ``ESP_ERR_`` value is found. This is done using ``strerror_r`` function. POSIX error codes (such as ``ENOENT``, ``ENOMEM``) are defined in ``errno.h`` and are typically obtained from ``errno`` variable. In ESP-IDF this variable is thread-local: multiple FreeRTOS tasks have their own copies of ``errno``. Functions which set ``errno`` only modify its value for the task they run in.

This feature is enabled by default, but can be disabled to reduce application binary size. See :ref:`CONFIG_ESP_ERR_TO_NAME_LOOKUP`. When this feature is disabled, :cpp:func:`esp_err_to_name` and :cpp:func:`esp_err_to_name_r` are still defined and can be called. In this case, :cpp:func:`esp_err_to_name` will return ``UNKNOWN ERROR``, and :cpp:func:`esp_err_to_name_r` will return ``Unknown error 0xXXXX(YYYYY)``, where ``0xXXXX`` and ``YYYYY`` are the hexadecimal and decimal representations of the error code, respectively.

.. _esp-error-check-macro:

Macro For Unrecoverable Errors
------------------------------

The :c:macro:`ESP_ERROR_CHECK` macro, defined in ``esp_err.h``, is used to handle unrecoverable errors in ESP-IDF applications. It functions similarly to the standard ``assert`` macro, but specifically checks whether an :cpp:type:`esp_err_t` value is equal to :c:macro:`ESP_OK`. If the value is not :c:macro:`ESP_OK`, :c:macro:`ESP_ERROR_CHECK` prints a detailed error message and calls ``abort()``, terminating the application.

The behavior of :c:macro:`ESP_ERROR_CHECK` can be controlled using assertion-related configuration options:

- If ``CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_ENABLE`` is set (default), the macro prints an error message and terminates the program.
- If ``CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT`` is enabled, the program terminates silently without printing an error message.
- If ``CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_DISABLE`` (``NDEBUG`` is defined), the macro only prints an error message and does not terminate the program.

Use :c:macro:`ESP_ERROR_CHECK` in situations where an error is considered fatal and the application cannot continue safely. For situations where the application can recover from an error, use the macros described in the next section.

``ESP_ERROR_CHECK_WITHOUT_ABORT``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :c:macro:`ESP_ERROR_CHECK_WITHOUT_ABORT` macro, defined in ``esp_err.h``, is closely related to the **Macros For Recoverable Errors**. The macro behaves similarly to :c:macro:`ESP_ERROR_CHECK`, but instead of terminating the program with ``abort()``, it prints an error message in the same format and returns the error code if the value is not :c:macro:`ESP_OK`. This allows the application to continue running, making it suitable for cases where errors should be reported but are not considered fatal.

The behavior of :c:macro:`ESP_ERROR_CHECK_WITHOUT_ABORT` is controlled by the same assertion-related configuration options as :c:macro:`ESP_ERROR_CHECK`. If either ``CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_DISABLE`` or ``CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT`` is enabled, the macro does not print any error message, otherwise, the macro prints an error message.

Use :c:macro:`ESP_ERROR_CHECK_WITHOUT_ABORT` when you want to print errors for diagnostic purposes without stopping the application.

Error message will typically look like this:

.. code-block:: none

    ESP_ERROR_CHECK failed: esp_err_t 0x107 (ESP_ERR_TIMEOUT) at 0x400d1fdf

    file: "/Users/user/esp/example/main/main.c" line 20
    func: app_main
    expression: sdmmc_card_init(host, &card)

    Backtrace: 0x40086e7c:0x3ffb4ff0 0x40087328:0x3ffb5010 0x400d1fdf:0x3ffb5030 0x400d0816:0x3ffb5050

.. note::

    If :doc:`ESP-IDF monitor <tools/idf-monitor>` is used, addresses in the backtrace will be converted to file names and line numbers.

- The first line mentions the error code as a hexadecimal value, and the identifier used for this error in source code. The latter depends on :ref:`CONFIG_ESP_ERR_TO_NAME_LOOKUP` option being set. Address in the program where error has occurred is printed as well.

- Subsequent lines show the location in the program where :c:macro:`ESP_ERROR_CHECK` macro was called, and the expression which was passed to the macro as an argument.

- Finally, backtrace is printed. This is part of panic handler output common to all fatal errors. See :doc:`Fatal Errors <fatal-errors>` for more information about the backtrace.


Macros For Recoverable Errors
-----------------------------

For recoverable errors, ESP-IDF provides a set of macros defined in ``esp_check.h``. The **ESP_RETURN_ON_...**, **ESP_GOTO_ON_...**, and **ESP_RETURN_VOID_ON_...** macros enable concise and consistent error handling, improving code readability and maintainability. Unlike ``ESP_ERROR_CHECK``, these macros do not terminate the program; instead, they print an error message and return or jump as appropriate. For use in interrupt service routines (ISRs), corresponding ``_ISR`` versions (such as :c:macro:`ESP_RETURN_ON_ERROR_ISR`) are available, ensuring safe operation in interrupt contexts.

The macros are defined as follows:

- **ESP_RETURN_ON_...**: Return from the function if an error or failed condition is detected:

    - :c:macro:`ESP_RETURN_ON_ERROR` - Checks an error code; if not :c:macro:`ESP_OK`, prints a message and returns the error code.
    - :c:macro:`ESP_RETURN_ON_FALSE` - Checks a condition; if false, prints a message and returns the supplied ``err_code``.
    - :c:macro:`ESP_RETURN_ON_ERROR_ISR` - For ISR context.
    - :c:macro:`ESP_RETURN_ON_FALSE_ISR` - For ISR context.

- **ESP_GOTO_ON_...**: Jump to a label if an error or failed condition is detected:

    - :c:macro:`ESP_GOTO_ON_ERROR` - Checks an error code; if not :c:macro:`ESP_OK`, prints a message, sets ``ret`` to the code, and jumps to ``goto_tag``.
    - :c:macro:`ESP_GOTO_ON_FALSE` - Checks a condition; if false, prints a message, sets ``ret`` to ``err_code``, and jumps to ``goto_tag``.
    - :c:macro:`ESP_GOTO_ON_ERROR_ISR` - For ISR context.
    - :c:macro:`ESP_GOTO_ON_FALSE_ISR` - For ISR context.

- **ESP_RETURN_VOID_...**: Return from a ``void`` function if an error or failed condition is detected:

    - :c:macro:`ESP_RETURN_VOID_ON_ERROR` - Checks an error code; if not :c:macro:`ESP_OK`, prints a message and returns.
    - :c:macro:`ESP_RETURN_VOID_ON_FALSE` - Checks a condition; if false, prints a message and returns.
    - :c:macro:`ESP_RETURN_VOID_ON_ERROR_ISR` - For ISR context.
    - :c:macro:`ESP_RETURN_VOID_ON_FALSE_ISR` - For ISR context.

The default behavior of these macros can be adjusted: if the :ref:`CONFIG_COMPILER_OPTIMIZATION_CHECKS_SILENT` option is enabled in Kconfig, error messages will not be included in the application binary and will not be printed.

.. _check_macros_examples:

Error Handling Examples
-----------------------

Some examples

.. code-block:: c

    static const char* TAG = "Test";

    esp_err_t test_func(void)
    {
        esp_err_t ret = ESP_OK;

        ESP_ERROR_CHECK(x);                                         // err message printed if `x` is not `ESP_OK`, and then `abort()`.
        ESP_ERROR_CHECK_WITHOUT_ABORT(x);                           // err message printed if `x` is not `ESP_OK`, without `abort()`.
        ESP_RETURN_ON_ERROR(x, TAG, "fail reason 1");               // err message printed if `x` is not `ESP_OK`, and then function returns with code `x`.
        ESP_GOTO_ON_ERROR(x, err, TAG, "fail reason 2");            // err message printed if `x` is not `ESP_OK`, `ret` is set to `x`, and then jumps to `err`.
        ESP_RETURN_ON_FALSE(a, err_code, TAG, "fail reason 3");     // err message printed if `a` is not `true`, and then function returns with code `err_code`.
        ESP_GOTO_ON_FALSE(a, err_code, err, TAG, "fail reason 4");  // err message printed if `a` is not `true`, `ret` is set to `err_code`, and then jumps to `err`.

    err:
        // clean up
        return ret;
    }

Error Handling Patterns
-----------------------

1. Attempt to recover. Depending on the situation, we may try the following methods:

    - retry the call after some time;
    - attempt to de-initialize the driver and re-initialize it again;
    - fix the error condition using an out-of-band mechanism (e.g reset an external peripheral which is not responding).

    Example:

    .. code-block:: c

        esp_err_t err;
        do {
            err = sdio_slave_send_queue(addr, len, arg, timeout);
            // keep retrying while the sending queue is full
        } while (err == ESP_ERR_TIMEOUT);
        if (err != ESP_OK) {
            // handle other errors
        }

2. Propagate the error to the caller. In some middleware components this means that a function must exit with the same error code, making sure any resource allocations are rolled back.

    Example:

    .. code-block:: c

        sdmmc_card_t* card = calloc(1, sizeof(sdmmc_card_t));
        if (card == NULL) {
            return ESP_ERR_NO_MEM;
        }
        esp_err_t err = sdmmc_card_init(host, &card);
        if (err != ESP_OK) {
            // Clean up
            free(card);
            // Propagate the error to the upper layer (e.g., to notify the user).
            // Alternatively, application can define and return custom error code.
            return err;
        }

3. Convert into unrecoverable error, for example using ``ESP_ERROR_CHECK``. See `Macro For Unrecoverable Errors`_ section for details.

    Terminating the application in case of an error is usually undesirable behavior for middleware components, but is sometimes acceptable at application level.

    Many ESP-IDF examples use ``ESP_ERROR_CHECK`` to handle errors from various APIs. This is not the best practice for applications, and is done to make example code more concise.

    Example:

    .. code-block:: c

        ESP_ERROR_CHECK(spi_bus_initialize(host, bus_config, dma_chan));


C++ Exceptions
--------------

See :ref:`cplusplus_exceptions`.

API Reference
-------------

See :ref:`esp-check-api-ref`.
````

## File: docs/en/api-guides/esp-wifi-mesh.rst
````
ESP-WIFI-MESH
================

:link_to_translation:`zh_CN:[中文]`

This guide provides information regarding the ESP-WIFI-MESH protocol. Please see the :doc:`ESP-WIFI-MESH API Reference <../api-reference/network/esp-wifi-mesh>` for more information about API usage.

.. ------------------------------- Overview -----------------------------------

Overview
--------

ESP-WIFI-MESH is a networking protocol built atop the Wi-Fi protocol. ESP-WIFI-MESH allows numerous devices (henceforth referred to as nodes) spread over a large physical area (both indoors and outdoors) to be interconnected under a single WLAN (Wireless Local-Area Network). ESP-WIFI-MESH is self-organizing and self-healing meaning the network can be built and maintained autonomously.

The ESP-WIFI-MESH guide is split into the following sections:

1. :ref:`mesh-introduction`

2. :ref:`mesh-concepts`

3. :ref:`mesh-building-a-network`

4. :ref:`mesh-managing-a-network`

5. :ref:`mesh-data-transmission`

6. :ref:`mesh-channel-switching`

7. :ref:`mesh-network-performance`

8. :ref:`mesh-further-notes`


.. ----------------------------- Introduction ---------------------------------

.. _mesh-introduction:

Introduction
------------

.. figure:: ../../_static/mesh-traditional-network-architecture.png
    :align: center
    :alt: Diagram of Traditional Network Architecture
    :figclass: align-center

    Traditional Wi-Fi Network Architecture

A traditional infrastructure Wi-Fi network is a point-to-multipoint network where a single central node known as the access point (AP) is directly connected to all other nodes known as stations. The AP is responsible for arbitrating and forwarding transmissions between the stations. Some APs also relay transmissions to/from an external IP network via a router. Traditional infrastructure Wi-Fi networks suffer the disadvantage of limited coverage area due to the requirement that every station must be in range to directly connect with the AP. Furthermore, traditional Wi-Fi networks are susceptible to overloading as the maximum number of stations permitted in the network is limited by the capacity of the AP.

.. figure:: ../../_static/mesh-esp-wifi-mesh-network-architecture.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Network Architecture
    :figclass: align-center

    ESP-WIFI-MESH Network Architecture

ESP-WIFI-MESH differs from traditional infrastructure Wi-Fi networks in that nodes are not required to connect to a central node. Instead, nodes are permitted to connect with neighboring nodes. Nodes are mutually responsible for relaying each others transmissions. This allows an ESP-WIFI-MESH network to have much greater coverage area as nodes can still achieve interconnectivity without needing to be in range of the central node. Likewise, ESP-WIFI-MESH is also less susceptible to overloading as the number of nodes permitted on the network is no longer limited by a single central node.


.. -------------------------- ESP-WIFI-MESH Concepts -------------------------------

.. _mesh-concepts:

ESP-WIFI-MESH Concepts
---------------------------

Terminology
^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Term
     - Description
   * - Node
     - Any device that **is** or **can be** part of an ESP-WIFI-MESH network
   * - Root Node
     - The top node in the network
   * - Child Node
     - A node X is a child node when it is connected to another node Y where the connection makes node X more distant from the root node than node Y (in terms of number of connections).
   * - Parent Node
     - The converse notion of a child node
   * - Descendant Node
     - Any node reachable by repeated proceeding from parent to child
   * - Sibling Nodes
     - Nodes that share the same parent node
   * - Connection
     - A traditional Wi-Fi association between an AP and a station. A node in ESP-WIFI-MESH will use its station interface to associate with the softAP interface of another node, thus forming a connection. The connection process includes the authentication and association processes in Wi-Fi.
   * - Upstream Connection
     - The connection from a node to its parent node
   * - Downstream Connection
     - The connection from a node to one of its child nodes
   * - Wireless Hop
     - The portion of the path between source and destination nodes that corresponds to a single wireless connection. A data packet that traverses a single connection is known as **single-hop** whereas traversing multiple connections is known as **multi-hop**.
   * - Subnetwork
     - A subnetwork is subdivision of an ESP-WIFI-MESH network which consists of a node and all of its descendant nodes. Therefore the subnetwork of the root node consists of all nodes in an ESP-WIFI-MESH network.
   * - MAC Address
     - Media Access Control Address used to uniquely identify each node or router within an ESP-WIFI-MESH network.
   * - DS
     - Distribution System (External IP Network)

Tree Topology
^^^^^^^^^^^^^

ESP-WIFI-MESH is built atop the infrastructure Wi-Fi protocol and can be thought of as a networking protocol that combines many individual Wi-Fi networks into a single WLAN. In Wi-Fi, stations are limited to a single connection with an AP (upstream connection) at any time, whilst an AP can be simultaneously connected to multiple stations (downstream connections). However ESP-WIFI-MESH allows nodes to simultaneously act as a station and an AP. Therefore a node in ESP-WIFI-MESH can have **multiple downstream connections using its softAP interface**, whilst simultaneously having **a single upstream connection using its station interface**. This naturally results in a tree network topology with a parent-child hierarchy consisting of multiple layers.

.. figure:: ../../_static/mesh-tree-topology.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Tree Topology
    :figclass: align-center

    ESP-WIFI-MESH Tree Topology

ESP-WIFI-MESH is a multiple hop (multi-hop) network meaning nodes can transmit packets to other nodes in the network through one or more wireless hops. Therefore, nodes in ESP-WIFI-MESH not only transmit their own packets, but simultaneously serve as relays for other nodes. Provided that a path exists between any two nodes on the physical layer (via one or more wireless hops), any pair of nodes within an ESP-WIFI-MESH network can communicate.

.. note::

    The size (total number of nodes) in an ESP-WIFI-MESH network is dependent on the maximum number of layers permitted in the network, and the maximum number of downstream connections each node can have. Both of these variables can be configured to limit the size of the network.

Node Types
^^^^^^^^^^

.. figure:: ../../_static/mesh-node-types.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Node Types
    :figclass: align-center

    ESP-WIFI-MESH Node Types

**Root Node:** The root node is the top node in the network and serves as the only interface between the ESP-WIFI-MESH network and an external IP network. The root node is connected to a conventional Wi-Fi router and relays packets to/from the external IP network to nodes within the ESP-WIFI-MESH network. **There can only be one root node within an ESP-WIFI-MESH network** and the root node's upstream connection may only be with the router. Referring to the diagram above, node A is the root node of the network.

**Leaf Nodes:** A leaf node is a node that is not permitted to have any child nodes (no downstream connections). Therefore a leaf node can only transmit or receive its own packets, but cannot forward the packets of other nodes. If a node is situated on the network's maximum permitted layer, it will be assigned as a leaf node. This prevents the node from forming any downstream connections thus ensuring the network does not add an extra layer. Some nodes without a softAP interface (station only) will also be assigned as leaf nodes due to the requirement of a softAP interface for any downstream connections. Referring to the diagram above, nodes L/M/N are situated on the networks maximum permitted layer hence have been assigned as leaf nodes .

**Intermediate Parent Nodes:** Connected nodes that are neither the root node or a leaf node are intermediate parent nodes. An intermediate parent node must have a single upstream connection (a single parent node), but can have zero to multiple downstream connections (zero to multiple child nodes). Therefore an intermediate parent node can transmit and receive packets, but also forward packets sent from its upstream and downstream connections. Referring to the diagram above, nodes B to J are intermediate parent nodes. **Intermediate parent nodes without downstream connections such as nodes E/F/G/I/J are not equivalent to leaf nodes** as they are still permitted to form downstream connections in the future.

**Idle Nodes:** Nodes that have yet to join the network are assigned as idle nodes. Idle nodes will attempt to form an upstream connection with an intermediate parent node or attempt to become the root node under the correct circumstances (see `Automatic Root Node Selection`_). Referring to the diagram above, nodes K and O are idle nodes.

Beacon Frames & RSSI Thresholding
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Every node in ESP-WIFI-MESH that is able to form downstream connections (i.e., has a softAP interface) will periodically transmit Wi-Fi beacon frames. A node uses beacon frames to allow other nodes to detect its presence and know of its status. Idle nodes will listen for beacon frames to generate a list of potential parent nodes, one of which the idle node will form an upstream connection with. ESP-WIFI-MESH uses the Vendor Information Element to store metadata such as:

- Node Type (Root, Intermediate Parent, Leaf, Idle)
- Current layer of Node
- Maximum number of layers permitted in the network
- Current number of child nodes
- Maximum number of downstream connections to accept

The signal strength of a potential upstream connection is represented by RSSI (Received Signal Strength Indication) of the beacon frames of the potential parent node. To prevent nodes from forming a weak upstream connection, ESP-WIFI-MESH implements an RSSI threshold mechanism for beacon frames. If a node detects a beacon frame with an RSSI below a preconfigured threshold, the transmitting node will be disregarded when forming an upstream connection.

.. figure:: ../../_static/mesh-beacon-frame-rssi.png
    :align: center
    :alt: Diagram of the Effects of RSSI Thresholding
    :figclass: align-center

    Effects of RSSI Thresholding

**Panel A** of the illustration above demonstrates how the RSSI threshold affects the number of parent node candidates an idle node has.

**Panel B** of the illustration above demonstrates how an RF shielding object can lower the RSSI of a potential parent node. Due to the RF shielding object, the area in which the RSSI of node X is above the threshold is significantly reduced. This causes the idle node to disregard node X even though node X is physically adjacent. The idle node will instead form an upstream connection with the physically distant node Y due to a stronger RSSI.

.. note::

    Nodes technically still receive all beacon frames on the MAC layer. The RSSI threshold is an ESP-WIFI-MESH feature that simply filters out all received beacon frames that are below the preconfigured threshold.

Preferred Parent Node
^^^^^^^^^^^^^^^^^^^^^

When an idle node has multiple parent nodes candidates (potential parent nodes), the idle node will form an upstream connection with the **preferred parent node**. The preferred parent node is determined based on the following criteria:

- Which layer the parent node candidate is situated on
- The number of downstream connections (child nodes) the parent node candidate currently has

The selection of the preferred parent node will always prioritize the parent node candidate on the shallowest layer of the network (including the root node). This helps minimize the total number of layers in an ESP-WIFI-MESH network when upstream connections are formed. For example, given a second layer node and a third layer node, the second layer node will always be preferred.

If there are multiple parent node candidates within the same layer, the parent node candidate with the least child nodes will be preferred. This criteria has the effect of balancing the number of downstream connections amongst nodes of the same layer.

.. figure:: ../../_static/mesh-preferred-parent-node.png
    :align: center
    :alt: Diagram of Preferred Parent Node Selection
    :figclass: align-center

    Preferred Parent Node Selection

**Panel A** of the illustration above demonstrates an example of how the idle node G selects a preferred parent node given the five parent node candidates B/C/D/E/F. Nodes on the shallowest layer are preferred, hence nodes B/C are prioritized since they are second layer nodes whereas nodes D/E/F are on the third layer. Node C is selected as the preferred parent node due it having fewer downstream connections (fewer child nodes) compared to node B.

**Panel B** of the illustration above demonstrates the case where the root node is within range of the idle node G. In other words, the root node's beacon frames are above the RSSI threshold when received by node G. The root node is always the shallowest node in an ESP-WIFI-MESH network hence is always the preferred parent node given multiple parent node candidates.

.. note::

    Users may also define their own algorithm for selecting a preferred parent node, or force a node to only connect with a specific parent node (see the :example:`Mesh Manual Networking Example <mesh/manual_networking>`).

Routing Tables
^^^^^^^^^^^^^^

Each node within an ESP-WIFI-MESH network will maintain its individual routing table used to correctly route ESP-WIFI-MESH packets (see `ESP-WIFI-MESH Packet`_) to the correct destination node. The routing table of a particular node will **consist of the MAC addresses of all nodes within the particular node's subnetwork** (including the MAC address of the particular node itself). Each routing table is internally partitioned into multiple subtables with each subtable corresponding to the subnetwork of each child node.

.. figure:: ../../_static/mesh-routing-tables-example.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Routing Tables Example
    :figclass: align-center

    ESP-WIFI-MESH Routing Tables Example

Using the diagram above as an example, the routing table of node B would consist of the MAC addresses of nodes B to I (i.e., equivalent to the subnetwork of node B). Node B's routing table is internally partitioned into two subtables containing of nodes C to F and nodes G to I (i.e., equivalent to the subnetworks of nodes C and G respectively).

**ESP-WIFI-MESH utilizes routing tables to determine whether an ESP-WIFI-MESH packet should be forwarded upstream or downstream based on the following rules.**

**1.** If the packet's destination MAC address is within the current node's routing table and is not the current node, select the subtable that contains the destination MAC address and forward the data packet downstream to the child node corresponding to the subtable.

**2.** If the destination MAC address is not within the current node's routing table, forward the data packet upstream to the current node's parent node. Doing so repeatedly will result in the packet arriving at the root node where the routing table should contain all nodes within the network.

.. note::

    Users can call :cpp:func:`esp_mesh_get_routing_table` to obtain a node's routing table, or :cpp:func:`esp_mesh_get_routing_table_size` to obtain the size of a node's routing table.  :cpp:func:`esp_mesh_get_subnet_nodes_list` can be used to obtain the corresponding subtable of a specific child node. Likewise :cpp:func:`esp_mesh_get_subnet_nodes_num` can be used to obtain the size of the subtable.


.. ------------------------ Building a Mesh Network ---------------------------

.. _mesh-building-a-network:

Building a Network
------------------

General Process
^^^^^^^^^^^^^^^

.. warning::

    Before the ESP-WIFI-MESH network building process can begin, certain parts of the configuration must be uniform across each node in the network (see :cpp:type:`mesh_cfg_t`). Each node must be configured with **the same Mesh Network ID, router configuration, and softAP configuration**.

An ESP-WIFI-MESH network building process involves selecting a root node, then forming downstream connections layer by layer until all nodes have joined the network. The exact layout of the network can be dependent on factors such as root node selection, parent node selection, and asynchronous power-on reset. However, the ESP-WIFI-MESH network building process can be generalized into the following steps:

.. figure:: ../../_static/mesh-network-building.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Network Building Process
    :figclass: align-center

    ESP-WIFI-MESH Network Building Process

1. Root Node Selection
""""""""""""""""""""""
The root node can be designated during configuration (see section on `User Designated Root Node`_), or dynamically elected based on the signal strength between each node and the router (see `Automatic Root Node Selection`_). Once selected, the root node will connect with the router and begin allowing downstream connections to form. Referring to the figure above, node A is selected to be the root node hence node A forms an upstream connection with the router.

2. Second Layer Formation
"""""""""""""""""""""""""
Once the root node has connected to the router, idle nodes in range of the root node will begin connecting with the root node thereby forming the second layer of the network. Once connected, the second layer nodes become intermediate parent nodes (assuming maximum permitted layers > 2) hence the next layer to form. Referring to the figure above, nodes B to D are in range of the root node. Therefore nodes B to D form upstream connections with the root node and become intermediate parent nodes.

3. Formation of Remaining Layers
""""""""""""""""""""""""""""""""
The remaining idle nodes will connect with intermediate parent nodes within range thereby forming a new layer in the network. Once connected, the idles nodes become intermediate parent node or leaf nodes depending on the networks maximum permitted layers. This step is repeated until there are no more idle nodes within the network or until the maximum permitted layer of the network has been reached. Referring to the figure above, nodes E/F/G connect with nodes B/C/D respectively and become intermediate parent nodes themselves.

4. Limiting Tree Depth
""""""""""""""""""""""
To prevent the network from exceeding the maximum permitted number of layers, nodes on the maximum layer will automatically become leaf nodes once connected. This prevents any other idle node from connecting with the leaf node thereby prevent a new layer form forming. However if an idle node has no other potential parent node, it will remain idle indefinitely. Referring to the figure above, the network's number of maximum permitted layers is set to four. Therefore when node H connects, it becomes a leaf node to prevent any downstream connections from forming.

Automatic Root Node Selection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The automatic selection of a root node involves an election process amongst all idle nodes based on their signal strengths with the router. Each idle node will transmit their MAC addresses and router RSSI values via Wi-Fi beacon frames. **The MAC address is used to uniquely identify each node in the network** whilst the **router RSSI** is used to indicate a node's signal strength with reference to the router.

Each node will then simultaneously scan for the beacon frames from other idle nodes. If a node detects a beacon frame with a stronger router RSSI, the node will begin transmitting the contents of that beacon frame (i.e., voting for the node with the stronger router RSSI). The process of transmission and scanning will repeat for a preconfigured minimum number of iterations (10 iterations by default) and result in the beacon frame with the strongest router RSSI being propagated throughout the network.

After all iterations, each node will individually check for its **vote percentage** (``number of votes/number of nodes participating in election``) to determine if it should become the root node. **If a node has a vote percentage larger than a preconfigured threshold (90% by default), the node will become a root node**.

The following diagram demonstrates how an ESP-WIFI-MESH network is built when the root node is automatically selected.

.. figure:: ../../_static/mesh-root-node-election-example.png
    :align: center
    :alt: Diagram of Root Node Election Process Example
    :figclass: align-center

    Root Node Election Example

**1.** On power-on reset, each node begins transmitting beacon frames consisting of their own MAC addresses and their router RSSIs.

**2.** Over multiple iterations of transmission and scanning, the beacon frame with the strongest router RSSI is propagated throughout the network. Node C has the strongest router RSSI (-10 dB) hence its beacon frame is propagated throughout the network. All nodes participating in the election vote for node C thus giving node C a vote percentage of 100%. Therefore node C becomes a root node and connects with the router.

**3.** Once Node C has connected with the router, nodes A/B/D/E connect with node C as it is the preferred parent node (i.e., the shallowest node). Nodes A/B/D/E form the second layer of the network.

**4.** Node F and G connect with nodes D and E respectively and the network building process is complete.

.. note::

    The minimum number of iterations for the election process can be configured using :cpp:func:`esp_mesh_set_attempts`. Users should adjust the number of iterations based on the number of nodes within the network (i.e., the larger the network the larger number of scan iterations required).

.. warning::

    **Vote percentage threshold** can also be configured using :cpp:func:`esp_mesh_set_vote_percentage`. Setting a low vote percentage threshold **can result in two or more nodes becoming root nodes** within the same ESP-WIFI-MESH network leading to the building of multiple networks. If such is the case, ESP-WIFI-MESH has internal mechanisms to autonomously resolve the **root node conflict**. The networks of the multiple root nodes will be combined into a single network with a single root node. However, root node conflicts where two or more root nodes have the same router SSID but different router BSSID are not handled.

User Designated Root Node
^^^^^^^^^^^^^^^^^^^^^^^^^

The root node can also be designated by user which will entail the designated root node to directly connect with the router and forgo the election process. When a root node is designated, all other nodes within the network must also forgo the election process to prevent the occurrence of a root node conflict. The following diagram demonstrates how an ESP-WIFI-MESH network is built when the root node is designated by the user.

.. figure:: ../../_static/mesh-root-node-designated-example.png
    :align: center
    :alt: Diagram of Root Node Designation Process Example
    :figclass: align-center

    Root Node Designation Example (Root Node = A, Max Layers = 4)

**1.** Node A is designated the root node by the user therefore directly connects with the router. All other nodes forgo the election process.

**2.** Nodes C/D connect with node A as their preferred parent node. Both nodes form the second layer of the network.

**3.** Likewise, nodes B/E connect with node C, and node F connects with node D. Nodes B/E/F form the third layer of the network.

**4.** Node G connects with node E, forming the fourth layer of the network. However the maximum permitted number of layers in this network is configured as four, therefore node G becomes a leaf node to prevent any new layers from forming.

.. note::

    When designating a root node, the root node should call :cpp:func:`esp_mesh_set_parent` in order to directly connect with the router. Likewise, all other nodes should call :cpp:func:`esp_mesh_fix_root` to forgo the election process.

Parent Node Selection
^^^^^^^^^^^^^^^^^^^^^

By default, ESP-WIFI-MESH is self organizing meaning that each node will autonomously select which potential parent node to form an upstream connection with. The autonomously selected parent node is known as the preferred parent node. The criteria used for selecting the preferred parent node is designed to reduce the number of layers in the ESP-WIFI-MESH network and to balance the number of downstream connections between potential parent nodes (see section on `Preferred Parent Node`_).

However ESP-WIFI-MESH also allows users to disable self-organizing behavior which will allow users to define their own criteria for parent node selection, or to configure nodes to have designated parent nodes (see the :example:`Mesh Manual Networking Example <mesh/manual_networking>`).

Asynchronous Power-on Reset
^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP-WIFI-MESH network building can be affected by the order in which nodes power-on. If certain nodes within the network power-on asynchronously (i.e., separated by several minutes), **the final structure of the network could differ from the ideal case where all nodes are powered on synchronously**. Nodes that are delayed in powering on will adhere to the following rules:

**Rule 1:** If a root node already exists in the network, the delayed node will not attempt to elect a new root node, even if it has a stronger RSSI with the router. The delayed node will instead join the network like any other idle node by connecting with a preferred parent node. If the delayed node is the designated root node, all other nodes in the network will remain idle until the delayed node powers-on.

**Rule 2:** If a delayed node forms an upstream connection and becomes an intermediate parent node, it may also become the new preferred parent of other nodes (i.e., being a shallower node). This will cause the other nodes to switch their upstream connections to connect with the delayed node (see `Parent Node Switching`_).

**Rule 3:** If an idle node has a designated parent node which is delayed in powering-on, the idle node will not attempt to form any upstream connections in the absence of its designated parent node. The idle node will remain idle indefinitely until its designated parent node powers-on.

The following example demonstrates the effects of asynchronous power-on with regards to network building.

.. figure:: ../../_static/mesh-asynchronous-power-on-example.png
    :align: center
    :alt: Diagram of Asynchronous Power On Example
    :figclass: align-center

    Network Building with Asynchronous Power On Example

**1.** Nodes A/C/D/F/G/H are powered-on synchronously and begin the root node election process by broadcasting their MAC addresses and router RSSIs. Node A is elected as the root node as it has the strongest RSSI.

**2.** Once node A becomes the root node, the remaining nodes begin forming upstream connections layer by layer with their preferred parent nodes. The result is a network with five layers.

**3.** Node B/E are delayed in powering-on but neither attempt to become the root node even though they have stronger router RSSIs (-20 dB and -10 dB) compared to node A. Instead both delayed nodes form upstream connections with their preferred parent nodes A and C respectively. Both nodes B/E become intermediate parent nodes after connecting.

**4.** Nodes D/G switch their upstream connections as node B is the new preferred parent node due to it being on a shallower layer (second layer node). Due to the switch, the resultant network has three layers instead of the original five layers.

**Synchronous Power-On:** Had all nodes powered-on synchronously, node E would have become the root node as it has the strongest router RSSI (-10 dB). This would result in a significantly different network layout compared to the network formed under the conditions of asynchronous power-on. **However the synchronous power-on network layout can still be reached if the user manually switches the root node** (see :cpp:func:`esp_mesh_waive_root`).

.. note::

    Differences in parent node selection caused by asynchronous power-on are  autonomously corrected for to some extent in ESP-WIFI-MESH (see `Parent Node Switching`_)

Loop-back Avoidance, Detection, and Handling
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A loop-back is the situation where a particular node forms an upstream connection with one of its descendant nodes (a node within the particular node's subnetwork). This results in a circular connection path thereby breaking the tree topology. ESP-WIFI-MESH prevents loop-back during parent selection by excluding nodes already present in the selecting node's routing table (see `Routing Tables`_) thus prevents a particular node from attempting to connect to any node within its subnetwork.

In the event that a loop-back occurs, ESP-WIFI-MESH utilizes a path verification mechanism and energy transfer mechanism to detect the loop-back occurrence. The parent node of the upstream connection that caused the loop-back will then inform the child node of the loop-back and initiate a disconnection.

.. -------------------------- Network Management ------------------------------

.. _mesh-managing-a-network:

Managing a Network
------------------

**ESP-WIFI-MESH is a self healing network meaning it can detect and correct for failures in network routing**. Failures occur when a parent node with one or more child nodes breaks down, or when the connection between a parent node and its child nodes becomes unstable. Child nodes in ESP-WIFI-MESH will autonomously select a new parent node and form an upstream connection with it to maintain network interconnectivity. ESP-WIFI-MESH can handle both Root Node Failures and Intermediate Parent Node Failures.

Root Node Failure
^^^^^^^^^^^^^^^^^

If the root node breaks down, the nodes connected with it (second layer nodes) will promptly detect the failure of the root node. The second layer nodes will initially attempt to reconnect with the root node. However after multiple failed attempts, the second layer nodes will initialize a new round of root node election. **The second layer node with the strongest router RSSI will be elected as the new root node** whilst the remaining second layer nodes will form an upstream connection with the new root node (or a neighboring parent node if not in range).

If the root node and multiple downstream layers simultaneously break down (e.g., root node, second layer, and third layer), the shallowest layer that is still functioning will initialize the root node election. The following example illustrates an example of self healing from a root node break down.

.. figure:: ../../_static/mesh-root-node-failure.png
    :align: center
    :alt: Diagram of Self Healing From Root Node Failure
    :figclass: align-center

    Self Healing From Root Node Failure

**1.** Node C is the root node of the network. Nodes A/B/D/E are second layer nodes connected to node C.

**2.** Node C breaks down. After multiple failed attempts to reconnect, the second layer nodes begin the election process by broadcasting their router RSSIs. Node B has the strongest router RSSI.

**3.** Node B is elected as the root node and begins accepting downstream connections. The remaining second layer nodes A/D/E form upstream connections with node B thus the network is healed and can continue operating normally.

.. note::

    If a designated root node breaks down, the remaining nodes **will not autonomously attempt to elect a new root node** as an election process will never be attempted whilst a designated root node is used.

Intermediate Parent Node Failure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If an intermediate parent node breaks down, the disconnected child nodes will initially attempt to reconnect with the parent node. After multiple failed attempts to reconnect, each child node will begin to scan for potential parent nodes (see `Beacon Frames & RSSI Thresholding`_).

If other potential parent nodes are available, each child node will individually select a new preferred parent node (see `Preferred Parent Node`_) and form an upstream connection with it. If there are no other potential parent nodes for a particular child node, it will remain idle indefinitely.

The following diagram illustrates an example of self healing from an Intermediate Parent Node break down.

.. figure:: ../../_static/mesh-parent-node-failure.png
    :align: center
    :alt: Diagram of Self Healing From Intermediate Parent Node Failure
    :figclass: align-center

    Self Healing From Intermediate Parent Node Failure

**1.** The following branch of the network consists of nodes A to G.

**2.** Node C breaks down. Nodes F/G detect the break down and attempt to reconnect with node C. After multiple failed attempts to reconnect, nodes F/G begin to select a new preferred parent node.

**3.** Node G is out of range from any other parent node hence remains idle for the time being. Node F is in range of nodes B/E, however node B is selected as it is the shallower node. Node F becomes an intermediate parent node after connecting with Node B thus node G can connect with node F. The network is healed, however the network routing as been affected and an extra layer has been added.

.. note::

    If a child node has a designated parent node that breaks down, the child node will make no attempt to connect with a new parent node. The child node will remain idle indefinitely.

Root Node Switching
^^^^^^^^^^^^^^^^^^^

ESP-WIFI-MESH does not automatically switch the root node unless the root node breaks down. Even if the root node's router RSSI degrades to the point of disconnection, the root node will remain unchanged. Root node switching is the act of explicitly starting a new election such that a node with a stronger router RSSI will be elected as the new root node. This can be a useful method of adapting to degrading root node performance.

To trigger a root node switch, the current root node must explicitly call :cpp:func:`esp_mesh_waive_root` to trigger a new election. The current root node will signal all nodes within the network to begin transmitting and scanning for beacon frames (see `Automatic Root Node Selection`_) **whilst remaining connected to the network (i.e., not idle)**. If another node receives more votes than the current root node, a root node switch will be initiated. **The root node will remain unchanged otherwise**.

A newly elected root node sends a **switch request** to the current root node which in turn will respond with an acknowledgment signifying both nodes are ready to switch. Once the acknowledgment is received, the newly elected root node will disconnect from its parent and promptly form an upstream connection with the router thereby becoming the new root node of the network. The previous root node will disconnect from the router **whilst maintaining all of its downstream connections** and enter the idle state. The previous root node will then begin scanning for potential parent nodes and selecting a preferred parent.

The following diagram illustrates an example of a root node switch.

.. figure:: ../../_static/mesh-root-node-switch-example.png
    :align: center
    :alt: Diagram of Root Node Switch Example
    :figclass: align-center

    Root Node Switch Example

**1.** Node C is the current root node but has degraded signal strength with the router (-85db). The node C triggers a new election and all nodes begin transmitting and scanning for beacon frames **whilst still being connected**.

**2.** After multiple rounds of transmission and scanning, node B is elected as the new root node. Node B sends node C a **switch request** and node C responds with an acknowledgment.

**3.** Node B disconnects from its parent and connects with the router becoming the network's new root node. Node C disconnects from the router, enters the idle state, and begins scanning for and selecting a new preferred parent node. **Node C maintains all its downstream connections throughout this process**.

**4.** Node C selects node B as its preferred parent node, forms an upstream connection, and becomes a second layer node. The network layout is similar after the switch as node C still maintains the same subnetwork. However each node in node C's subnetwork has been placed one layer deeper as a result of the switch. `Parent Node Switching`_ may adjust the network layout afterwards if any nodes have a new preferred parent node as a result of the root node switch.

.. note::

    Root node switching must require an election hence is only supported when using a self-organized ESP-WIFI-MESH network. In other words, root node switching cannot occur if a designated root node is used.

Parent Node Switching
^^^^^^^^^^^^^^^^^^^^^

Parent Node Switching entails a child node switching its upstream connection to another parent node of a shallower layer. **Parent Node Switching occurs autonomously** meaning that a child node will change its upstream connection automatically if a potential parent node of a shallower layer becomes available (i.e., due to a `Asynchronous Power-on Reset`_).

All potential parent nodes periodically transmit beacon frames (see `Beacon Frames & RSSI Thresholding`_) allowing for a child node to scan for the availability of a shallower parent node. Due to parent node switching, a self-organized ESP-WIFI-MESH network can dynamically adjust its network layout to ensure each connection has a good RSSI and that the number of layers in the network is minimized.


.. --------------------------- Data Transmission ------------------------------

.. _mesh-data-transmission:

Data Transmission
-----------------

ESP-WIFI-MESH Packet
^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP-WIFI-MESH network data transmissions use ESP-WIFI-MESH packets. ESP-WIFI-MESH packets are **entirely contained within the frame body of a Wi-Fi data frame**. A multi-hop data transmission in an ESP-WIFI-MESH network will involve a single ESP-WIFI-MESH packet being carried over each wireless hop by a different Wi-Fi data frame.

The following diagram shows the structure of an ESP-WIFI-MESH packet and its relation with a Wi-Fi data frame.

.. figure:: ../../_static/mesh-packet.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Packet
    :figclass: align-center

    ESP-WIFI-MESH Packet

**The header** of an ESP-WIFI-MESH packet contains the MAC addresses of the source and destination nodes. The options field contains information pertaining to the special types of ESP-WIFI-MESH packets such as a group transmission or a packet originating from the external IP network (see :c:macro:`MESH_OPT_SEND_GROUP` and :c:macro:`MESH_OPT_RECV_DS_ADDR`).

**The payload** of an ESP-WIFI-MESH packet contains the actual application data. This data can be raw binary data, or encoded under an application layer protocol such as HTTP, MQTT, and JSON (see :cpp:type:`mesh_proto_t`).

.. note::

    When sending an ESP-WIFI-MESH packet to the external IP network, the destination address field of the header will contain the IP address and port of the target server rather than the MAC address of a node (see :cpp:type:`mesh_addr_t`). Furthermore the root node will handle the formation of the outgoing TCP/IP packet.

Group Control & Multicasting
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Multicasting is a feature that allows a single ESP-WIFI-MESH packet to be transmitted simultaneously to multiple nodes within the network. Multicasting in ESP-WIFI-MESH can be achieved by either specifying a list of target nodes, or specifying a preconfigured group of nodes. Both methods of multicasting are called via :cpp:func:`esp_mesh_send`.

To multicast by specifying a list of target nodes, users must first set the ESP-WIFI-MESH packet's destination address to the **Multicast-Group Address** (``01:00:5E:xx:xx:xx``). This signifies that the ESP-WIFI-MESH packet is a multicast packet with a group of addresses, and that the address should be obtained from the header options. Users must then list the MAC addresses of the target nodes as options (see :cpp:type:`mesh_opt_t` and :c:macro:`MESH_OPT_SEND_GROUP`). This method of multicasting requires no prior setup but can incur a large amount of overhead data as each target node's MAC address must be listed in the options field of the header.

Multicasting by group allows a ESP-WIFI-MESH packet to be transmitted to a preconfigured group of nodes. Each grouping is identified by a unique ID, and a node can be placed into a group via :cpp:func:`esp_mesh_set_group_id`. Multicasting to a group involves setting the destination address of the ESP-WIFI-MESH packet to the target group ID. Furthermore, the :c:macro:`MESH_DATA_GROUP` flag must set. Using groups to multicast incurs less overhead, but requires nodes to previously added into groups.

.. note::

    During a multicast, all nodes within the network still receive the ESP-WIFI-MESH packet on the MAC layer. However, nodes not included in the MAC address list or the target group will simply filter out the packet.

Broadcasting
^^^^^^^^^^^^

Broadcasting is a feature that allows a single ESP-WIFI-MESH packet to be transmitted simultaneously to all nodes within the network. Each node essentially forwards a broadcast packet to all of its upstream and downstream connections such that the packet propagates throughout the network as quickly as possible. However, ESP-WIFI-MESH utilizes the following methods to avoid wasting bandwidth during a broadcast.

**1.** When an intermediate parent node receives a broadcast packet from its parent, it will forward the packet to each of its child nodes whilst storing a copy of the packet for itself.

**2.** When an intermediate parent node is the source node of the broadcast, it will transmit the broadcast packet upstream to is parent node and downstream to each of its child nodes.

**3.** When an intermediate parent node receives a broadcast packet from one of its child nodes, it will forward the packet to its parent node and each of its remaining child nodes whilst storing a copy of the packet for itself.

**4.** When a leaf node is the source node of a broadcast, it will directly transmit the packet to its parent node.

**5.** When the root node is the source node of a broadcast, the root node will transmit the packet to all of its child nodes.

**6.** When the root node receives a broadcast packet from one of its child nodes, it will forward the packet to each of its remaining child nodes whilst storing a copy of the packet for itself.

**7.** When a node receives a broadcast packet with a source address matching its own MAC address, the node will discard the broadcast packet.

**8.** When an intermediate parent node receives a broadcast packet from its parent node which was originally transmitted from one of its child nodes, it will discard the broadcast packet

Upstream Flow Control
^^^^^^^^^^^^^^^^^^^^^

ESP-WIFI-MESH relies on parent nodes to control the upstream data flow of their immediate child nodes. To prevent a parent node's message buffer from overflowing due to an overload of upstream transmissions, a parent node will allocate a quota for upstream transmissions known as a **receiving window** for each of its child nodes. **Each child node must apply for a receiving window before it is permitted to transmit upstream**. The size of a receiving window can be dynamically adjusted. An upstream transmission from a child node to the parent node consists of the following steps:

**1.** Before each transmission, the child node sends a window request to its parent node. The window request consists of a sequence number which corresponds to the child node's data packet that is pending transmission.

**2.** The parent node receives the window request and compares the sequence number with the sequence number of the previous packet sent by the child node. The comparison is used to calculate the size of the receiving window which is transmitted back to the child node.

**3.** The child node transmits the data packet in accordance with the window size specified by the parent node. If the child node depletes its receiving window, it must obtain another receiving windows by sending a request before it is permitted to continue transmitting.

.. note::

    ESP-WIFI-MESH does not support any downstream flow control.

.. warning::

    Due to `Parent Node Switching`_, packet loss may occur during upstream transmissions.

Due to the fact that the root node acts as the sole interface to an external IP network, it is critical that downstream nodes are aware of the root node's connection status with the external IP network. Failing to do so can lead to nodes attempting to pass data upstream to the root node whilst it is disconnected from the IP network. This results in unnecessary transmissions and packet loss. ESP-WIFI-MESH address this issue by providing a mechanism to stabilize the throughput of outgoing data based on the connection status between the root node and the external IP network. The root node can broadcast its external IP network connection status to all other nodes by calling :cpp:func:`esp_mesh_post_toDS_state`.

Bi-Directional Data Stream
^^^^^^^^^^^^^^^^^^^^^^^^^^

The following diagram illustrates the various network layers involved in an ESP-WIFI-MESH Bidirectional Data Stream.

.. figure:: ../../_static/mesh-bidirectional-data-stream.png
    :align: center
    :alt: Diagram of ESP-WIFI-MESH Bidirectional Data Stream
    :figclass: align-center

    ESP-WIFI-MESH Bidirectional Data Stream

Due to the use of `Routing Tables`_, **ESP-WIFI-MESH is able to handle pack forwarding entirely on the mesh layer**. A TCP/IP layer is only required on the root node when it transmits/receives a packet to/from an external IP network.


.. --------------------------- Channel Switching -------------------------------

.. _mesh-channel-switching:

Channel Switching
-----------------

Background
^^^^^^^^^^

In traditional Wi-Fi networks, **channels** are predetermined frequency ranges. In an infrastructure basic service set (BSS), the serving AP and its connected stations must be on the same operating channels (1 to 14) in which beacons are transmitted. Physically adjacent BSS (Basic Service Sets) operating on the same channel can lead to interference and degraded performance.

In order to allow a BSS adapt to changing physical layer conditions and maintain performance, Wi-Fi contains mechanisms for **network channel switching**. A network channel switch is an attempt to move a BSS to a new operating channel whilst minimizing disruption to the BSS during this process. However it should be recognized that a channel switch may be unsuccessful in  moving all stations to the new operating channel.

In an infrastructure Wi-Fi network, network channel switches are triggered by the AP with the aim of having the AP and all connected stations synchronously switch to a new channel. Network channel switching is implemented by embedding a **Channel Switch Announcement (CSA)** element within the AP's periodically transmitted beacon frames. The CSA element is used to advertise to all connected stations regarding an upcoming network channel switch and will be included in multiple beacon frames up until the switch occurs.

A CSA element contains information regarding the **New Channel Number** and a **Channel Switch Count** which indicates the number of beacon frame intervals (TBTTs) remaining until the network channel switch occurs. Therefore, the Channel Switch Count is decremented every beacon frame and allows connected stations to synchronize their channel switch with the AP.

ESP-WIFI-MESH Network Channel Switching
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP-WIFI-MESH Network Channel Switching also utilize beacon frames that contain a CSA element. However, being a multi-hop network makes the switching process in ESP-WIFI-MESH is more complex due to the fact that a beacon frame might not be able to reach all nodes within the network (i.e., in a single hop). Therefore, an ESP-WIFI-MESH network relies on nodes to forward the CSA element so that it is propagated throughout the network.

When an intermediate parent node with one or more child nodes receives a beacon frame containing a CSA, the node will forward the CSA element by including the element in its next transmitted beacon frame (i.e., with the same **New Channel Number** and **Channel Switch Count**). Given that all nodes within an ESP-WIFI-MESH network receive the same CSA, the nodes can synchronize their channel switches using the Channel Switch Count, albeit with a short delay due to CSA element forwarding.

An ESP-WIFI-MESH network channel switch can be triggered by either the router or the root node.

Root Node Triggered
"""""""""""""""""""
**A root node triggered channel switch can only occur when the ESP-WIFI-MESH network is not connected to a router**. By calling :cpp:func:`esp_mesh_switch_channel`, the root node will set an initial Channel Switch Count value and begin including a CSA element in its beacon frames. Each CSA element is then received by second layer nodes, and forwarded downstream in their own beacon frames.

Router Triggered
""""""""""""""""
When an ESP-WIFI-MESH network is connected to a router, the entire network must use the same channel as the router. Therefore, **the root node will not be permitted to trigger a channel switch when it is connected to a router**.

When the root node receives beacon frame containing a CSA element from the router, **the root node will set Channel Switch Count value in the CSA element to a custom value before forwarding it downstream via beacon frames**. It will also decrement the Channel Switch Count of subsequent CSA elements relative to the custom value. This custom value can be based on factors such as the number of network layers, the current number of nodes etc.

The setting the Channel Switch Count value to a custom value is due to the fact that the ESP-WIFI-MESH network and its router may have a different and varying beacon intervals. Therefore, the Channel Switch Count value provided by the router is irrelevant to an ESP-WIFI-MESH network. By using a custom value, nodes within the ESP-WIFI-MESH network are able to switch channels synchronously relative to the ESP-WIFI-MESH network's beacon interval. However, this will also result in the ESP-WIFI-MESH network's channel switch being unsynchronized with the channel switch of the router and its connected stations.

Impact of Network Channel Switching
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Due to the ESP-WIFI-MESH network channel switch being unsynchronized with the router's channel switch, there will be a **temporary channel discrepancy** between the ESP-WIFI-MESH network and the router.
    - The ESP-WIFI-MESH network's channel switch time is dependent on the ESP-WIFI-MESH network's beacon interval and the root node's custom Channel Switch Count value.
    - The channel discrepancy prevents any data exchange between the root node and the router during that ESP-WIFI-MESH network's switch.
    - In the ESP-WIFI-MESH network, the root node and intermediate parent nodes will request their connected child nodes to stop transmissions until the channel switch takes place by setting the **Channel Switch Mode** field in the CSA element to 1.
    - Frequent router triggered network channel switches can degrade the ESP-WIFI-MESH network's performance. Note that this can be caused by the ESP-WIFI-MESH network itself (e.g., due to wireless medium contention with ESP-WIFI-MESH network). If this is the case, users should disable the automatic channel switching on the router and use a specified channel instead.

- When there is a **temporary channel discrepancy**, the root node remains technically connected to the router.
    - Disconnection occurs after the root node fails to receive any beacon frames or probe responses from the router over a fixed number of router beacon intervals.
    - Upon disconnection, the root node will automatically re-scan all channels for the presence of a router.

- If the root node is unable to receive any of the router's CSA beacon frames (e.g., due to short switch time given by the router), the router will switch channels without the ESP-WIFI-MESH network's knowledge.
    - After the router switches channels, the root node will no longer be able to receive the router's beacon frames and probe responses and result in a disconnection after a fixed number of beacon intervals.
    - The root node will re-scan all channels for the router after disconnection.
    - The root node will maintain downstream connections throughout this process.

.. note::

    Although ESP-WIFI-MESH network channel switching aims to move all nodes within the network to a new operating channel, it should be recognized that a channel switch might not successfully move all nodes (e.g., due to reasons such as node failures).

Channel and Router Switching Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP-WIFI-MESH allows for autonomous channel switching to be enabled/disabled via configuration. Likewise, autonomous router switching (i.e., when a root node autonomously connects to another router) can also be enabled/disabled by configuration. Autonomous channel switching and router switching is dependent on the following configuration parameters and run-time conditions.

**Allow Channel Switch:** This parameter is set via the ``allow_channel_switch`` field of the :cpp:type:`mesh_cfg_t` structure and permits an ESP-WIFI-MESH network to dynamically switch channels when set.

**Preset Channel:** An ESP-WIFI-MESH network can have a preset channel by setting the ``channel`` field of the :cpp:type:`mesh_cfg_t` structure to the desired channel number. If this field is unset, the ``allow_channel_switch`` parameter is overridden such that channel switches are always permitted.

**Allow Router Switch:** This parameter is set via the ``allow_router_switch`` field of the :cpp:type:`mesh_router_t` and permits an ESP-WIFI-MESH to dynamically switch to a different router when set.

**Preset Router BSSID:** An ESP-WIFI-MESH network can have a preset router by setting the ``bssid`` field of the :cpp:type:`mesh_router_t` structure to the
BSSID of the desired router. If this field is unset, the ``allow_router_switch`` parameter is overridden such that router switches are always permitted.

**Root Node Present:** The presence of a root node will can also affect whether or a channel or router switch is permitted.

The following table illustrates how the different combinations of parameters/conditions affect whether channel switching and/or router switching is permitted. Note that `X` represents a "do not care" for the parameter.

.. list-table::
    :widths: 15 15 15 15 15 15
    :header-rows: 1

    * - Preset Channel
      - Allow Channel Switch
      - Preset Router BSSID
      - Allow Router Switch
      - Root Node Present
      - Permitted Switches？
    * - N
      - X
      - N
      - X
      - X
      - Channel and Router
    * - N
      - X
      - Y
      - N
      - X
      - Channel Only
    * - N
      - X
      - Y
      - Y
      - X
      - Channel and Router
    * - Y
      - Y
      - N
      - X
      - X
      - Channel and Router
    * - Y
      - N
      - N
      - X
      - N
      - Router Only
    * - Y
      - N
      - N
      - X
      - Y
      - Channel and Router
    * - Y
      - Y
      - Y
      - N
      - X
      - Channel Only
    * - Y
      - N
      - Y
      - N
      - N
      - N
    * - Y
      - N
      - Y
      - N
      - Y
      - Channel Only
    * - Y
      - Y
      - Y
      - Y
      - X
      - Channel and Router
    * - Y
      - N
      - Y
      - Y
      - N
      - Router Only
    * - Y
      - N
      - Y
      - Y
      - Y
      - Channel and Router

.. ------------------------------ Performance ---------------------------------

.. _mesh-network-performance:

Performance
-----------

The performance of an ESP-WIFI-MESH network can be evaluated based on multiple metrics such as the following:

**Network Building Time:** The amount of time taken to build an ESP-WIFI-MESH network from scratch.

**Healing Time:** The amount of time taken for the network to detect a node break down and carry out appropriate actions to heal the network (such as generating a new root node or forming new connections).

**Per-hop latency:** The latency of data transmission over one wireless hop. In other words, the time taken to transmit a data packet from a parent node to a child node or vice versa.

**Network Node Capacity:** The total number of nodes the ESP-WIFI-MESH network can simultaneously support. This number is determined by the maximum number of downstream connections a node can accept and the maximum number of layers permissible in the network.

The following table lists the common performance figures of an ESP-WIFI-MESH network:

* Network Building Time: < 60 seconds
* Healing time:
    * Root node break down: < 10 seconds
    * Child node break down: < 5 seconds
* Per-hop latency: 10 to 30 milliseconds

.. note::

    The following test conditions were used to generate the performance figures above.

    - Number of test devices: **100**
    - Maximum Downstream Connections to Accept: **6**
    - Maximum Permissible Layers: **6**

.. note::

    Throughput depends on packet error rate and hop count.

.. note::

    The throughput of root node's access to the external IP network is directly affected by the number of nodes in the ESP-WIFI-MESH network and the bandwidth of the router.

.. note::

    The performance figures can vary greatly between installations based on network configuration and operating environment.

.. ----------------------------- Further Notes --------------------------------

.. _mesh-further-notes:

Further Notes
-------------

- Data transmission uses Wi-Fi WPA2-PSK encryption

- Mesh networking IE uses AES encryption


Router and internet icon made by `Smashicons <https://smashicons.com>`_ from `www.flaticon.com <https://smashicons.com>`_
````

## File: docs/en/api-guides/external-ram.rst
````
Support for External RAM
************************

:link_to_translation:`zh_CN:[中文]`

.. toctree::
    :maxdepth: 1

Introduction
============

{IDF_TARGET_PSRAM_VADDR_SIZE:default="Value not updated", esp32="4 MB", esp32s2="10.5 MB", esp32s3="32 MB", esp32p4="64 MB"}

{IDF_TARGET_NAME} has a few hundred kilobytes of internal RAM, residing on the same die as the rest of the chip components. It can be insufficient for some purposes, so {IDF_TARGET_NAME} has the ability to use up to {IDF_TARGET_PSRAM_VADDR_SIZE} of virtual addresses for external PSRAM (pseudo-static RAM) memory. The external memory is incorporated in the memory map and, with certain restrictions, is usable in the same way as internal data RAM.

.. only:: esp32s3

    The {IDF_TARGET_PSRAM_VADDR_SIZE} virtual addresses are shared with flash instructions and rodata.

Hardware
========

{IDF_TARGET_NAME} supports PSRAM connected in parallel with the SPI flash chip. While {IDF_TARGET_NAME} is capable of supporting several types of RAM chips, ESP-IDF currently only supports Espressif branded PSRAM chips (e.g., ESP-PSRAM32, ESP-PSRAM64, etc).

.. note::

    .. only:: esp32

        Some PSRAM chips are 1.8 V devices and some are 3.3 V. The working voltage of the PSRAM chip must match the working voltage of the flash component. Consult the datasheet for your PSRAM chip and {IDF_TARGET_NAME} device to find out the working voltages. For a 1.8 V PSRAM chip, make sure to either set the MTDI pin to a high signal level on boot-up, or program {IDF_TARGET_NAME} eFuses to always use the VDD_SIO level of 1.8 V. Not doing this can damage the PSRAM and/or flash chip.

    .. only:: esp32s2 or esp32s3

        Some PSRAM chips are 1.8 V devices and some are 3.3 V. The working voltage of the PSRAM chip must match the working voltage of the flash component. Consult the datasheet for your PSRAM chip and {IDF_TARGET_NAME} device to find out the working voltages. For a 1.8 V PSRAM chip, make sure to either set the GPIO45 strapping pin to a high signal level on boot-up, or program {IDF_TARGET_NAME} eFuses to always use the VDD_SPI level of 1.8 V. Not doing this can damage the PSRAM and/or flash chip.

    .. only:: esp32p4

        Some PSRAM chips are 1.8 V devices and some are 3.3 V. Consult the datasheet for your PSRAM chip and {IDF_TARGET_NAME} device to find out the working voltages.

        By default, the PSRAM is powered up by the on-chip LDO2. You can use :ref:`CONFIG_ESP_LDO_CHAN_PSRAM_DOMAIN` to switch the LDO channel accordingly. Set this value to -1 to use an external power supply, which means the on-chip LDO will not be used. By default, the PSRAM connected to LDO is set to the correct voltage based on the Espressif module used. You can still use :ref:`CONFIG_ESP_LDO_VOLTAGE_PSRAM_DOMAIN` to select the LDO output voltage if you are not using an Espressif module. When using an external power supply, this option does not exist.

.. note::

    Espressif produces both modules and system-in-package chips that integrate compatible PSRAM and flash and are ready to mount on a product PCB. Consult the Espressif website for more information. If you are using a custom PSRAM chip, ESP-IDF SDK might not be compatible with it.

For specific details about connecting the SoC or module pins to an external PSRAM chip, consult the SoC or module datasheet.

.. _external_ram_config:


Configuring External RAM
========================

.. note::

    The ``SPI RAM`` configuration options are available only if the ``esp_psram`` component is included in the build. To include ``SPI RAM`` into your project, add the ``esp_psram`` component as a dependency in either ``REQUIRES`` or ``PRIV_REQUIRES`` when registering your component with ``idf_component_register``.

ESP-IDF fully supports the use of external RAM in applications. Once the external RAM is initialized at startup, ESP-IDF can be configured to integrate the external RAM in several ways:

.. list::

    * :ref:`external_ram_config_memory_map`
    * :ref:`external_ram_config_capability_allocator`
    * :ref:`external_ram_config_malloc` (default)
    * :ref:`external_ram_config_bss`
    * :ref:`external_ram_config_noinit`
    :SOC_SPIRAM_XIP_SUPPORTED: * :ref:`external_ram_config_xip`

.. _external_ram_config_memory_map:


Integrate RAM into the {IDF_TARGET_NAME} Memory Map
---------------------------------------------------

Select this option by choosing ``Integrate RAM into memory map`` from :ref:`CONFIG_SPIRAM_USE`.

This is the most basic option for external RAM integration. Most likely, you will need another, more advanced option.

During the ESP-IDF startup, external RAM is mapped into the data virtual address space. The address space is dynamically allocated. The length will be the minimum length between the PSRAM size and the available data virtual address space size.

Applications can manually place data in external memory by creating pointers to this region. So if an application uses external memory, it is responsible for all management of the external RAM: coordinating buffer usage, preventing corruption, etc.

It is recommended to access the PSRAM by ESP-IDF heap memory allocator (see next chapter).

.. _external_ram_config_capability_allocator:


Add External RAM to the Capability Allocator
--------------------------------------------

Select this option by choosing ``Make RAM allocatable using heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`` from :ref:`CONFIG_SPIRAM_USE`.

When enabled, memory is mapped to data virtual address space and also added to the :doc:`capabilities-based heap memory allocator </api-reference/system/mem_alloc>` using ``MALLOC_CAP_SPIRAM``.

To allocate memory from external RAM, a program should call ``heap_caps_malloc(size, MALLOC_CAP_SPIRAM)``. After use, this memory can be freed by calling the normal ``free()`` function.

.. _external_ram_config_malloc:


Provide External RAM via malloc()
---------------------------------

Select this option by choosing ``Make RAM allocatable using malloc() as well`` from :ref:`CONFIG_SPIRAM_USE`. This is the default option.

In this case, memory is added to the capability allocator as described for the previous option. However, it is also added to the pool of RAM that can be returned by the standard ``malloc()`` function.

This allows any application to use the external RAM without having to rewrite the code to use ``heap_caps_malloc(..., MALLOC_CAP_SPIRAM)``.

An additional configuration item, :ref:`CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL`, can be used to set the size threshold when a single allocation should prefer external memory:

- When allocating a size less than or equal to the threshold, the allocator will try internal memory first.
- When allocating a size larger than the threshold, the allocator will try external memory first.

If a suitable block of preferred internal/external memory is not available, the allocator will try the other type of memory.

Because some buffers can only be allocated in internal memory, a second configuration item :ref:`CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL` defines a pool of internal memory which is reserved for *only* explicitly internal allocations (such as memory for DMA use). Regular ``malloc()`` will not allocate from this pool. The :ref:`MALLOC_CAP_DMA <dma-capable-memory>` and ``MALLOC_CAP_INTERNAL`` flags can be used to allocate memory from this pool.

.. _external_ram_config_bss:

Allow .bss Segment to Be Placed in External Memory
--------------------------------------------------

Enable this option by checking :ref:`CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY`.

If enabled, the region of the data virtual address space where the PSRAM is mapped to will be used to store zero-initialized data (BSS segment) from the lwIP, net80211, libpp, wpa_supplicant and bluedroid ESP-IDF libraries.

Additional data can be moved from the internal BSS segment to external RAM by applying the macro ``EXT_RAM_BSS_ATTR`` to any static declaration (which is not initialized to a non-zero value).

It is also possible to place the BSS section of a component or a library to external RAM using linker fragment scheme ``extram_bss``.

This option reduces the internal static memory used by the BSS segment.

Remaining external RAM can also be added to the capability heap allocator using the method shown above.

.. _external_ram_config_noinit:

Allow .noinit Segment to Be Placed in External Memory
--------------------------------------------------------------

Enable this option by checking :ref:`CONFIG_SPIRAM_ALLOW_NOINIT_SEG_EXTERNAL_MEMORY`. If enabled, the region of the data virtual address space where the PSRAM is mapped to will be used to store non-initialized data. The values placed in this segment will not be initialized or modified even during startup or restart.

By applying the macro ``EXT_RAM_NOINIT_ATTR``, data could be moved from the internal NOINIT segment to external RAM. Remaining external RAM can still be added to the capability heap allocator using the method shown above, :ref:`external_ram_config_capability_allocator`.

.. only:: SOC_SPIRAM_XIP_SUPPORTED

    .. only:: esp32s2 or esp32s3

        Move Instructions in Flash to PSRAM
        -----------------------------------

        The :ref:`CONFIG_SPIRAM_FETCH_INSTRUCTIONS` option allows the flash ``.text`` sections (for instructions) to be placed in PSRAM.

        By enabling the :ref:`CONFIG_SPIRAM_FETCH_INSTRUCTIONS` option,

        - Instructions from the ``.text`` sections of flash are moved into PSRAM on system startup.

        - The corresponding virtual memory range of those instructions will also be re-mapped to PSRAM.

        Move Read-Only Data in Flash to PSRAM
        ---------------------------------------

        The :ref:`CONFIG_SPIRAM_RODATA` option allows the flash ``.rodata`` sections (for read only data) to be placed in PSRAM.

        By enabling the :ref:`CONFIG_SPIRAM_RODATA` option,

        - Instructions from the ``.rodata`` sections of flash are moved into PSRAM on system startup.

        - The corresponding virtual memory range of those rodata will also be re-mapped to PSRAM.

        .. _external_ram_config_xip:

        Execute In Place (XiP) from PSRAM
        ------------------------------------

        The :ref:`CONFIG_SPIRAM_XIP_FROM_PSRAM` is a helper option for you to select both the :ref:`CONFIG_SPIRAM_FETCH_INSTRUCTIONS` and :ref:`CONFIG_SPIRAM_RODATA`.

        The benefits of XiP from PSRAM is:

        - PSRAM access speed may be faster than flash access, so the overall application performance may be better. For example, if the PSRAM is an Octal mode (8-line PSRAM) and is configured to 80 MHz, then it is faster than a Quad flash (4-line flash) which is configured to 80 MHz.

        - The cache will not be disabled during an SPI1 flash operation, thus optimizing the code execution performance during SPI1 flash operations. For ISRs, ISR callbacks and data which might be accessed during this period, you do not need to place them in internal RAM, thus internal RAM usage can be optimized. This feature is useful for high throughput peripheral involved applications to improve the performance during SPI1 flash operations.

        :example:`system/xip_from_psram` demonstrates the usage of XiP from PSRAM, optimizing internal RAM usage and avoiding cache disabling during flash operations from user call (e.g., flash erase/read/write operations).

    .. only:: not (esp32s2 or esp32s3)

        .. _external_ram_config_xip:

        Execute In Place (XiP) from PSRAM
        ------------------------------------

        The :ref:`CONFIG_SPIRAM_XIP_FROM_PSRAM` option enables the executable in place (XiP) from PSRAM feature. With this option sections that are normally placed in flash, ``.text`` (for instructions) and ``.rodata`` (for read only data), will be loaded in PSRAM.

        With this option enabled, the cache will not be disabled during an SPI1 flash operation, so code that requires executing during an SPI1 flash operation does not have to be placed in internal RAM.

        .. only:: SOC_MMU_PER_EXT_MEM_TARGET

            Since the flash and PSRAM in {IDF_TARGET_NAME} use two separate SPI buses, moving flash content to PSRAM will actually increase the load on the PSRAM MSPI bus. Therefore, the exact impact on performance will be dependent on your app usage of PSRAM.

            The PSRAM bus can operate at a higher speed than the flash bus. For example, if the PSRAM is a HEX (16-line PSRAM on ESP32P4) running at 200 MHz, it is significantly faster than a Quad flash (4-line flash) running at 80 MHz.

            If the instructions and data previously stored in flash are not accessed frequently, then enabling this option could improve performance. It is recommended to conduct performance profiling to evaluate how this option will affect your system.

Restrictions
============

External RAM use has the following restrictions:

.. list::

    - When flash cache is disabled (for example, if the flash is being written to), the external RAM also becomes inaccessible. Any read operations from or write operations to it will lead to an illegal cache access exception. This is also the reason why ESP-IDF does not by default allocate any task stacks in external RAM (see below).

    :esp32s2: - External RAM cannot be used as a place to store DMA transaction descriptors or as a buffer for a DMA transfer to read from or write into. Therefore when External RAM is enabled, any buffer that will be used in combination with DMA must be allocated using ``heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)`` and can be freed using a standard ``free()`` call. Note that although {IDF_TARGET_NAME} has hardware support for DMA to or from external RAM, this is not yet supported in ESP-IDF.

    :esp32s3: - Although {IDF_TARGET_NAME} has hardware support for DMA to or from external RAM, there are still limitations:

        :esp32s3: - DMA transaction descriptors cannot be placed in PSRAM.
        :esp32s3: - The bandwidth that DMA accesses external RAM is very limited, especially when the core is trying to access the external RAM at the same time.
        :esp32s3: - You can configure :ref:`CONFIG_SPIRAM_SPEED` as 120 MHz for an octal PSRAM. The bandwidth will be improved. However there are still restrictions for this option. See :ref:`All Supported PSRAM Modes and Speeds <flash-psram-combination>` for more details.

    - External RAM uses the same cache region as the external flash. This means that frequently accessed variables in external RAM can be read and modified almost as quickly as in internal RAM. However, when accessing large chunks of data (> 32 KB), the cache can be insufficient, and speeds will fall back to the access speed of the external RAM. Moreover, accessing large chunks of data can "push out" cached flash, possibly making the execution of code slower afterwards.

    - In general, external RAM will not be used as task stack memory. :cpp:func:`xTaskCreate` and similar functions will always allocate internal memory for stack and task TCBs.

The option :ref:`CONFIG_FREERTOS_TASK_CREATE_ALLOW_EXT_MEM` can be used to allow placing task stacks into external memory. In these cases :cpp:func:`xTaskCreateStatic` must be used to specify a task stack buffer allocated from external memory, otherwise task stacks will still be allocated from internal memory.


Failure to Initialize
=====================

By default, failure to initialize external RAM will cause the ESP-IDF startup to abort. This can be disabled by enabling the config item :ref:`CONFIG_SPIRAM_IGNORE_NOTFOUND`.

.. only:: esp32 or esp32s2

    If :ref:`CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` is enabled, the option to ignore failure is not available as the linker will have assigned symbols to external memory addresses at link time.


.. only:: not esp32

    Encryption
    ==========

    It is possible to enable automatic encryption for data stored in external RAM. When this is enabled any data read and written through the cache will automatically be encrypted or decrypted by the external memory encryption hardware.

    This feature is enabled whenever flash encryption is enabled. For more information on how to enable and how it works see :doc:`Flash Encryption </security/flash-encryption>`.


.. only:: esp32

    .. include:: inc/external-ram-esp32-notes.rst

.. _ESP32 Series SoC Errata: https://www.espressif.com/sites/default/files/documentation/eco_and_workarounds_for_bugs_in_esp32_en.pdf
.. _ESP32 Chip Revision v3.0 User Guide: https://www.espressif.com/sites/default/files/documentation/ESP32_ECO_V3_User_Guide__EN.pdf
````

## File: docs/en/api-guides/fatal-errors.rst
````
Fatal Errors
============

:link_to_translation:`zh_CN:[中文]`

.. _Overview:

Overview
--------

In certain situations, the execution of the program can not be continued in a well-defined way. In ESP-IDF, these situations include:

- CPU Exceptions: |CPU_EXCEPTIONS_LIST|
- System level checks and safeguards:

    .. list::

        - :doc:`Interrupt watchdog <../api-reference/system/wdts>` timeout
        - :doc:`Task watchdog <../api-reference/system/wdts>` timeout (only fatal if :ref:`CONFIG_ESP_TASK_WDT_PANIC` is set)
        - Cache access error
        :SOC_MEMPROT_SUPPORTED: - Memory protection fault
        - Brownout detection event
        - Stack overflow
        - Stack smashing protection check
        - Heap integrity check
        - Undefined behavior sanitizer (UBSAN) checks

- Failed assertions, via ``assert``, ``configASSERT`` and :doc:`similar macros </api-guides/error-handling>`.

This guide explains the procedure used in ESP-IDF for handling these errors, and provides suggestions on troubleshooting the errors.

.. _Panic-Handler:

Panic Handler
-------------

Every error cause listed in the :ref:`Overview` will be handled by the *panic handler*.

The panic handler will start by printing the cause of the error to the console. For CPU exceptions, the message will be similar to

.. parsed-literal::

    Guru Meditation Error: Core 0 panic'ed (|ILLEGAL_INSTR_MSG|). Exception was unhandled.

For some of the system level checks (interrupt watchdog, cache access error), the message will be similar to

.. parsed-literal::

    Guru Meditation Error: Core 0 panic'ed (|CACHE_ERR_MSG|). Exception was unhandled.

In all cases, the error cause will be printed in parentheses. See :ref:`Guru-Meditation-Errors` for a list of possible error causes.

Subsequent behavior of the panic handler can be set using :ref:`CONFIG_ESP_SYSTEM_PANIC` configuration choice. The available options are:

- Print registers and reboot (``CONFIG_ESP_SYSTEM_PANIC_PRINT_REBOOT``) — default option.

    This will print register values at the point of the exception, print the backtrace, and restart the chip.

- Print registers and halt (``CONFIG_ESP_SYSTEM_PANIC_PRINT_HALT``)

    Similar to the above option, but halt instead of rebooting. External reset is required to restart the program.

- Silent reboot (``CONFIG_ESP_SYSTEM_PANIC_SILENT_REBOOT``)

    Do not print registers or backtrace, restart the chip immediately.

- Invoke GDB Stub (``CONFIG_ESP_SYSTEM_PANIC_GDBSTUB``)

    Start GDB server which can communicate with GDB over console UART port. This option will only provide read-only debugging or post-mortem debugging. See `GDB Stub`_ for more details.

.. note::

    The ``CONFIG_ESP_SYSTEM_PANIC_GDBSTUB`` choice in the configuration option :ref:`CONFIG_ESP_SYSTEM_PANIC` is only available when the component ``esp_gdbstub`` is included in the build.

The behavior of the panic handler is affected by three other configuration options.

- If :ref:`CONFIG_ESP_DEBUG_OCDAWARE` is enabled (which is the default), the panic handler will detect whether a JTAG debugger is connected. If it is, execution will be halted and control will be passed to the debugger. In this case, registers and backtrace are not dumped to the console, and GDBStub / Core Dump functions are not used.

- If the :doc:`Core Dump <core_dump>` feature is enabled, then the system state (task stacks and registers) will be dumped to either Flash or UART, for later analysis.

- If :ref:`CONFIG_ESP_PANIC_HANDLER_IRAM` is disabled (disabled by default), the panic handler code is placed in flash memory, not IRAM. This means that if ESP-IDF crashes while flash cache is disabled, the panic handler will automatically re-enable flash cache before running GDB Stub or Core Dump. This adds some minor risk, if the flash cache status is also corrupted during the crash.

    If this option is enabled, the panic handler code (including required UART functions) is placed in IRAM, and hence will decrease the usable memory space in SRAM. But this may be necessary to debug some complex issues with crashes while flash cache is disabled (for example, when writing to SPI flash) or when flash cache is corrupted when an exception is triggered.

- If :ref:`CONFIG_ESP_SYSTEM_PANIC_REBOOT_DELAY_SECONDS` is enabled (disabled by default) and set to a number higher than 0, the panic handler will delay the reboot for that amount of time in seconds. This can help if the tool used to monitor serial output does not provide a possibility to stop and examine the serial output. In that case, delaying the reboot will allow users to examine and debug the panic handler output (backtrace, etc.) for the duration of the delay. After the delay, the device will reboot. The reset reason is preserved.

The following diagram illustrates the panic handler behavior:

.. blockdiag::
    :scale: 100%
    :caption: Panic Handler Flowchart (click to enlarge)
    :align: center

    blockdiag panic-handler {
        orientation = portrait;
        edge_layout = flowchart;
        default_group_color = white;
        node_width = 160;
        node_height = 60;

        cpu_exception [label = "CPU Exception", shape=roundedbox];
        sys_check [label = "Cache error,\nInterrupt WDT,\nabort()", shape=roundedbox];
        check_ocd [label = "JTAG debugger\nconnected?", shape=diamond, height=80];
        print_error_cause [label = "Print error/\nexception cause"];
        use_jtag [label = "Send signal to\nJTAG debugger", shape=roundedbox];
        dump_registers [label = "Print registers\nand backtrace"];
        check_coredump [label = "Core dump\nenabled?", shape=diamond, height=80];
        do_coredump [label = "Core dump\nto UART or Flash"];
        check_gdbstub [label = "GDB Stub\nenabled?", shape=diamond, height=80];
        do_gdbstub [label = "Start GDB Stub", shape=roundedbox];
        halt [label = "Halt", shape=roundedbox];
        reboot [label = "Reboot", shape=roundedbox];
        check_halt [label = "Halt?", shape=diamond, height=80];

        group {cpu_exception, sys_check};

        cpu_exception -> print_error_cause;
        sys_check -> print_error_cause;
        print_error_cause -> check_ocd;
        check_ocd -> use_jtag [label = "Yes"];
        check_ocd -> dump_registers [label = "No"];
        dump_registers -> check_coredump
        check_coredump -> do_coredump [label = "Yes"];
        do_coredump -> check_gdbstub;
        check_coredump -> check_gdbstub [label = "No"];
        check_gdbstub -> check_halt [label = "No"];
        check_gdbstub -> do_gdbstub [label = "Yes"];
        check_halt -> halt [label = "Yes"];
        check_halt -> reboot [label = "No"];
    }

Register Dump and Backtrace
---------------------------

Unless the ``CONFIG_ESP_SYSTEM_PANIC_SILENT_REBOOT`` option is enabled, the panic handler prints some of the CPU registers, and the backtrace, to the console.

.. only:: CONFIG_IDF_TARGET_ARCH_XTENSA

    .. code-block:: none

        Core 0 register dump:
        PC      : 0x400e14ed  PS      : 0x00060030  A0      : 0x800d0805  A1      : 0x3ffb5030
        A2      : 0x00000000  A3      : 0x00000001  A4      : 0x00000001  A5      : 0x3ffb50dc
        A6      : 0x00000000  A7      : 0x00000001  A8      : 0x00000000  A9      : 0x3ffb5000
        A10     : 0x00000000  A11     : 0x3ffb2bac  A12     : 0x40082d1c  A13     : 0x06ff1ff8
        A14     : 0x3ffb7078  A15     : 0x00000000  SAR     : 0x00000014  EXCCAUSE: 0x0000001d
        EXCVADDR: 0x00000000  LBEG    : 0x4000c46c  LEND    : 0x4000c477  LCOUNT  : 0xffffffff

        Backtrace: 0x400e14ed:0x3ffb5030 0x400d0802:0x3ffb5050

.. only:: CONFIG_IDF_TARGET_ARCH_RISCV

    .. code-block:: none

        Core  0 register dump:
        MEPC    : 0x420048b4  RA      : 0x420048b4  SP      : 0x3fc8f2f0  GP      : 0x3fc8a600
        TP      : 0x3fc8a2ac  T0      : 0x40057fa6  T1      : 0x0000000f  T2      : 0x00000000
        S0/FP   : 0x00000000  S1      : 0x00000000  A0      : 0x00000001  A1      : 0x00000001
        A2      : 0x00000064  A3      : 0x00000004  A4      : 0x00000001  A5      : 0x00000000
        A6      : 0x42001fd6  A7      : 0x00000000  S2      : 0x00000000  S3      : 0x00000000
        S4      : 0x00000000  S5      : 0x00000000  S6      : 0x00000000  S7      : 0x00000000
        S8      : 0x00000000  S9      : 0x00000000  S10     : 0x00000000  S11     : 0x00000000
        T3      : 0x00000000  T4      : 0x00000000  T5      : 0x00000000  T6      : 0x00000000
        MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000007  MTVAL   : 0x00000000
        MHARTID : 0x00000000

The register values printed are the register values in the exception frame, i.e., values at the moment when the CPU exception or another fatal error has occurred.

A Register dump is not printed if the panic handler has been executed as a result of an ``abort()`` call.

.. only:: CONFIG_IDF_TARGET_ARCH_XTENSA

    In some cases, such as interrupt watchdog timeout, the panic handler may print additional CPU registers (EPC1-EPC4) and the registers/backtrace of the code running on the other CPU.

    The backtrace line contains PC:SP pairs, where PC is the Program Counter and SP is Stack Pointer, for each stack frame of the current task. If a fatal error happens inside an ISR, the backtrace may include PC:SP pairs both from the task which was interrupted, and from the ISR.

If :doc:`IDF Monitor <tools/idf-monitor>` is used, Program Counter values will be converted to code locations (function name, file name, and line number), and the output will be annotated with additional lines:

.. only:: CONFIG_IDF_TARGET_ARCH_XTENSA

    .. code-block:: none

        Core 0 register dump:
        PC      : 0x400e14ed  PS      : 0x00060030  A0      : 0x800d0805  A1      : 0x3ffb5030
        0x400e14ed: app_main at /Users/user/esp/example/main/main.cpp:36

        A2      : 0x00000000  A3      : 0x00000001  A4      : 0x00000001  A5      : 0x3ffb50dc
        A6      : 0x00000000  A7      : 0x00000001  A8      : 0x00000000  A9      : 0x3ffb5000
        A10     : 0x00000000  A11     : 0x3ffb2bac  A12     : 0x40082d1c  A13     : 0x06ff1ff8
        0x40082d1c: _calloc_r at /Users/user/esp/esp-idf/components/newlib/syscalls.c:51

        A14     : 0x3ffb7078  A15     : 0x00000000  SAR     : 0x00000014  EXCCAUSE: 0x0000001d
        EXCVADDR: 0x00000000  LBEG    : 0x4000c46c  LEND    : 0x4000c477  LCOUNT  : 0xffffffff

        Backtrace: 0x400e14ed:0x3ffb5030 0x400d0802:0x3ffb5050
        0x400e14ed: app_main at /Users/user/esp/example/main/main.cpp:36

        0x400d0802: main_task at /Users/user/esp/esp-idf/components/{IDF_TARGET_PATH_NAME}/cpu_start.c:470

.. only:: CONFIG_IDF_TARGET_ARCH_RISCV

    .. code-block:: none

        Core  0 register dump:
        MEPC    : 0x420048b4  RA      : 0x420048b4  SP      : 0x3fc8f2f0  GP      : 0x3fc8a600
        0x420048b4: app_main at /Users/user/esp/example/main/hello_world_main.c:20

        0x420048b4: app_main at /Users/user/esp/example/main/hello_world_main.c:20

        TP      : 0x3fc8a2ac  T0      : 0x40057fa6  T1      : 0x0000000f  T2      : 0x00000000
        S0/FP   : 0x00000000  S1      : 0x00000000  A0      : 0x00000001  A1      : 0x00000001
        A2      : 0x00000064  A3      : 0x00000004  A4      : 0x00000001  A5      : 0x00000000
        A6      : 0x42001fd6  A7      : 0x00000000  S2      : 0x00000000  S3      : 0x00000000
        0x42001fd6: uart_write at /Users/user/esp/esp-idf/components/vfs/vfs_uart.c:201

        S4      : 0x00000000  S5      : 0x00000000  S6      : 0x00000000  S7      : 0x00000000
        S8      : 0x00000000  S9      : 0x00000000  S10     : 0x00000000  S11     : 0x00000000
        T3      : 0x00000000  T4      : 0x00000000  T5      : 0x00000000  T6      : 0x00000000
        MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000007  MTVAL   : 0x00000000
        MHARTID : 0x00000000

    Moreover, :doc:`IDF Monitor <tools/idf-monitor>` is also capable of generating and printing a backtrace thanks to the stack dump provided by the board in the panic handler.
    The output looks like this:

    .. code-block:: none

        Backtrace:

        0x42006686 in bar (ptr=ptr@entry=0x0) at ../main/hello_world_main.c:18
        18      *ptr = 0x42424242;
        #0  0x42006686 in bar (ptr=ptr@entry=0x0) at ../main/hello_world_main.c:18
        #1  0x42006692 in foo () at ../main/hello_world_main.c:22
        #2  0x420066ac in app_main () at ../main/hello_world_main.c:28
        #3  0x42015ece in main_task (args=<optimized out>) at /Users/user/esp/components/freertos/port/port_common.c:142
        #4  0x403859b8 in vPortEnterCritical () at /Users/user/esp/components/freertos/port/riscv/port.c:130
        #5  0x00000000 in ?? ()
        Backtrace stopped: frame did not save the PC

    While the backtrace above is very handy, it requires the user to use :doc:`IDF Monitor <tools/idf-monitor>`. Thus, in order to generate and print a backtrace while using another monitor program, it is possible to activate ``CONFIG_ESP_SYSTEM_USE_EH_FRAME`` option from the menuconfig, under the "Backtracing method" menu.

    This option will let the compiler generate DWARF information for each function of the project. Then, when a CPU exception occurs, the panic handler will parse these data and determine the backtrace of the task that failed. The output looks like this:

    .. code-block:: none

        Backtrace: 0x42009e9a:0x3fc92120 0x42009ea6:0x3fc92120 0x42009ec2:0x3fc92130 0x42024620:0x3fc92150 0x40387d7c:0x3fc92160 0xfffffffe:0x3fc92170

    These ``PC:SP`` pairs represent the PC (Program Counter) and SP (Stack Pointer) for each stack frame of the current task.


    The main benefit of the ``CONFIG_ESP_SYSTEM_USE_EH_FRAME`` option is that the backtrace is generated by the board itself (without the need for :doc:`IDF Monitor <tools/idf-monitor>`). However, the option's drawback is that it results in an increase of the compiled binary's size (ranging from 20% to 100% increase in size). Furthermore, this option causes debug information to be included within the compiled binary. Therefore, users are strongly advised not to enable this option in mass production builds.

    Another option to generate such backtrace on the device itself is to enable ``CONFIG_ESP_SYSTEM_USE_FRAME_POINTER`` option from the menuconfig, under the "Backtracing method" menu.

    This option will let the compiler reserve a CPU register that keeps track of the frame of each routine of the program. This registers makes it possible for the panic handler to unwind the call stack at any given time, and more importantly, when a CPU exception occurs.

    Enabling ``CONFIG_ESP_SYSTEM_USE_FRAME_POINTER`` option will result in an increase of the compiled binary's size of around +5-6% and a performance decrease of around 1%. Contrarily to the ``CONFIG_ESP_SYSTEM_USE_EH_FRAME`` option, the compiler won't generate debug information in the generated binary, so it is possible to use this feature in mass production builds.

To find the location where a fatal error has happened, look at the lines which follow the "Backtrace" line. Fatal error location is the top line, and subsequent lines show the call stack.

.. _GDB-Stub:

GDB Stub
--------

If the ``CONFIG_ESP_SYSTEM_PANIC_GDBSTUB`` option is enabled, the panic handler will not reset the chip when a fatal error happens. Instead, it will start a GDB remote protocol server, commonly referred to as GDB Stub. When this happens, a GDB instance running on the host computer can be instructed to connect to the {IDF_TARGET_NAME} UART port.

If :doc:`IDF Monitor <tools/idf-monitor>` is used, GDB is started automatically when a GDB Stub prompt is detected on the UART. The output looks like this:

.. code-block:: none

    Entering gdb stub now.
    $T0b#e6GNU gdb (crosstool-NG crosstool-ng-1.22.0-80-gff1f415) 7.10
    Copyright (C) 2015 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
    and "show warranty" for details.
    This GDB was configured as "--host=x86_64-build_apple-darwin16.3.0 --target={IDF_TARGET_TOOLCHAIN_PREFIX}".
    Type "show configuration" for configuration details.
    For bug reporting instructions, please see:
    <http://www.gnu.org/software/gdb/bugs/>.
    Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.
    For help, type "help".
    Type "apropos word" to search for commands related to "word"...
    Reading symbols from /Users/user/esp/example/build/example.elf...done.
    Remote debugging using /dev/cu.usbserial-31301
    0x400e1b41 in app_main ()
        at /Users/user/esp/example/main/main.cpp:36
    36      *((int*) 0) = 0;
    (gdb)

The GDB prompt can be used to inspect CPU registers, local and static variables, and arbitrary locations in memory. It is not possible to set breakpoints, change the PC, or continue execution. To reset the program, exit GDB and perform an external reset: Ctrl-T Ctrl-R in IDF Monitor, or using the external reset button on the development board.

.. _RTC-Watchdog-Timeout:

RTC Watchdog Timeout
--------------------

{IDF_TARGET_RTCWDT_RTC_RESET:default="Not updated", esp32="RTCWDT_RTC_RESET", esp32s2="RTCWDT_RTC_RST", esp32s3="RTCWDT_RTC_RST", esp32c3="RTCWDT_RTC_RST", esp32c2="RTCWDT_RTC_RST", esp32c6="LP_WDT_SYS", esp32h2="LP_WDT_SYS", esp32p4="LP_WDT_SYS"}

The RTC watchdog is used in the startup code to keep track of execution time and it also helps to prevent a lock-up caused by an unstable power source. It is enabled by default (see :ref:`CONFIG_BOOTLOADER_WDT_ENABLE`). If the execution time is exceeded, the RTC watchdog will restart the system. In this case, the first stage (ROM) bootloader will print a message with the ``RTC Watchdog Timeout`` reason for the reboot.

.. code-block:: none

    rst:0x10 ({IDF_TARGET_RTCWDT_RTC_RESET})


The RTC watchdog covers the execution time from the first stage (ROM) bootloader to application startup. It is initially set in the first stage (ROM) bootloader, then configured in the bootloader with the :ref:`CONFIG_BOOTLOADER_WDT_TIME_MS` option (9000 ms by default). During the application initialization stage, it is reconfigured because the source of the slow clock may have changed, and finally disabled right before the ``app_main()`` call. There is an option :ref:`CONFIG_BOOTLOADER_WDT_DISABLE_IN_USER_CODE` which prevents the RTC watchdog from being disabled before ``app_main``. Instead, the RTC watchdog remains active and must be fed periodically in your application's code.

The RTC watchdog is also used by the system :ref:`panic handler <Panic-Handler>` to protect the system from hanging during a panic. The RTC watchdog is reconfigured in the panic handler to have a timeout of 10 seconds. If the panic handler takes longer than 10 seconds to execute, the system will be reset by the RTC watchdog.

.. _Guru-Meditation-Errors:

Guru Meditation Errors
----------------------

.. Note to editor: titles of the following section need to match exception causes printed by the panic handler. Do not change the titles (insert spaces, reword, etc.) unless the panic handler messages are also changed.

.. Note to translator: When translating this section, avoid translating the following section titles. "Guru Meditation" in the title of this section should not be translated either. Keep these two notes when translating.

This section explains the meaning of different error causes, printed in parens after the ``Guru Meditation Error: Core panic'ed`` message.

.. note::

    See the `Guru Meditation Wikipedia article <https://en.wikipedia.org/wiki/Guru_Meditation>`_ for historical origins of "Guru Meditation".


|ILLEGAL_INSTR_MSG|
^^^^^^^^^^^^^^^^^^^

This CPU exception indicates that the instruction which was executed was not a valid instruction. The most common reasons for this error include:

- FreeRTOS task function has returned. In FreeRTOS, if a task function needs to terminate, it should call :cpp:func:`vTaskDelete` and delete itself, instead of returning.

- Failure to read next instruction from SPI flash. This usually happens if:

    - Application has reconfigured the SPI flash pins as some other function (GPIO, UART, etc.). Consult the Hardware Design Guidelines and the datasheet for the chip or module for details about the SPI flash pins.

    - Some external device has accidentally been connected to the SPI flash pins, and has interfered with communication between {IDF_TARGET_NAME} and SPI flash.

- In C++ code, exiting from a non-void function without returning a value is considered to be an undefined behavior. When optimizations are enabled, the compiler will often omit the epilogue in such functions. This most often results in an |ILLEGAL_INSTR_MSG| exception. By default, ESP-IDF build system enables ``-Werror=return-type`` which means that missing return statements are treated as compile time errors. However if the application project disables compiler warnings, this issue might go undetected and the |ILLEGAL_INSTR_MSG| exception will occur at run time.

.. only:: CONFIG_IDF_TARGET_ARCH_XTENSA

    InstrFetchProhibited
    ^^^^^^^^^^^^^^^^^^^^

    This CPU exception indicates that the CPU could not read an instruction because the address of the instruction does not belong to a valid region in instruction RAM or ROM.

    Usually, this means an attempt to call a function pointer, which does not point to valid code. ``PC`` (Program Counter) register can be used as an indicator: it will be zero or will contain a garbage value (not ``0x4xxxxxxx``).

    LoadProhibited, StoreProhibited
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    These CPU exceptions happen when an application attempts to read from or write to an invalid memory location. The address which has been written/read is found in the ``EXCVADDR`` register in the register dump. If this address is zero, it usually means that the application has attempted to dereference a NULL pointer. If this address is close to zero, it usually means that the application has attempted to access a member of a structure, but the pointer to the structure is NULL. If this address is something else (garbage value, not in ``0x3fxxxxxx`` - ``0x6xxxxxxx`` range), it likely means that the pointer used to access the data is either not initialized or has been corrupted.

    IntegerDivideByZero
    ^^^^^^^^^^^^^^^^^^^

    Application has attempted to do an integer division by zero.

    LoadStoreAlignment
    ^^^^^^^^^^^^^^^^^^

    Application has attempted to read or write a memory location, and the address alignment does not match the load/store size. For example, a 32-bit read can only be done from a 4-byte aligned address, and a 16-bit write can only be done to a 2-byte aligned address.

    LoadStoreError
    ^^^^^^^^^^^^^^

    This exception may happen in the following cases:

    - If the application has attempted to do an 8- or 16- bit read to, or write from, a memory region which only supports 32-bit reads/writes. For example, dereferencing a ``char*`` pointer to instruction memory (IRAM, IROM) will result in such an error.

    - If the application has attempted to write to a read-only memory region, such as IROM or DROM.

    Unhandled Debug Exception
    ^^^^^^^^^^^^^^^^^^^^^^^^^

    This CPU exception happens when the instruction ``BREAK`` is executed.

.. only:: CONFIG_IDF_TARGET_ARCH_RISCV

    Instruction Address Misaligned
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    This CPU exception indicates that the address of the instruction to execute is not 2-byte aligned.

    Instruction Access Fault, Load Access Fault, Store Access Fault
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    This CPU exception happens when application attempts to execute, read from or write to an invalid memory location. The address which was written/read is found in ``MTVAL`` register in the register dump. If this address is zero, it usually means that application attempted to dereference a NULL pointer. If this address is close to zero, it usually means that application attempted to access member of a structure, but the pointer to the structure was NULL. If this address is something else (garbage value, not in ``0x3fxxxxxx`` - ``0x6xxxxxxx`` range), it likely means that the pointer used to access the data was either not initialized or was corrupted.

    Breakpoint
    ^^^^^^^^^^

    This CPU exception happens when the instruction ``EBREAK`` is executed. See also :ref:`FreeRTOS-End-Of-Stack-Watchpoint`.

    Load Address Misaligned, Store Address Misaligned
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Application has attempted to read or write memory location, and address alignment did not match load/store size. For example, 32-bit load can only be done from 4-byte aligned address, and 16-bit load can only be done from a 2-byte aligned address.

Interrupt Watchdog Timeout on CPU0/CPU1
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Indicates that an interrupt watchdog timeout has occurred. See :doc:`Watchdogs <../api-reference/system/wdts>` for more information.

.. _cache_error:

|CACHE_ERR_MSG|
^^^^^^^^^^^^^^^

In some situations, ESP-IDF will temporarily disable access to external SPI flash and SPI RAM via caches. For example, this happens when spi_flash APIs are used to read/write/erase/mmap regions of SPI flash. In these situations, tasks are suspended, and interrupt handlers not registered with ``ESP_INTR_FLAG_IRAM`` are disabled. Make sure that any interrupt handlers registered with this flag have all the code and data in IRAM/DRAM. For more details, see the :ref:`SPI flash API documentation <iram-safe-interrupt-handlers>` and the :ref:`IRAM-Safe Interrupt Handlers <iram_safe_interrupts_handlers>` section.

.. only:: SOC_MEMPROT_SUPPORTED

    Memory Protection Fault
    ^^^^^^^^^^^^^^^^^^^^^^^

    {IDF_TARGET_NAME} Permission Control feature is used in ESP-IDF to prevent the following types of memory access:

    * writing to instruction RAM after the program is loaded
    * executing code from data RAM (areas used for heap and static .data and .bss)

    Such operations are not necessary for most programs. Prohibiting such operations typically makes software vulnerabilities harder to exploit. Applications which rely on dynamic loading or self-modifying code may disable this protection using :ref:`CONFIG_ESP_SYSTEM_MEMPROT` Kconfig option.

    When the fault occurs, the panic handler reports the address of the fault and the type of memory access that caused it.

Other Fatal Errors
------------------

.. only:: SOC_BOD_SUPPORTED

    Brownout
    ^^^^^^^^

    {IDF_TARGET_NAME} has a built-in brownout detector, which is enabled by default. The brownout detector can trigger a system reset if the supply voltage goes below a safe level. The brownout detector can be configured using :ref:`CONFIG_ESP_BROWNOUT_DET` and :ref:`CONFIG_ESP_BROWNOUT_DET_LVL_SEL` options.

    When the brownout detector triggers, the following message is printed:

    .. code-block:: none

        Brownout detector was triggered

    The chip is reset after the message is printed.

    Note that if the supply voltage is dropping at a fast rate, only part of the message may be seen on the console.


Corrupt Heap
^^^^^^^^^^^^

ESP-IDF's heap implementation contains a number of run-time checks of the heap structure. Additional checks ("Heap Poisoning") can be enabled in menuconfig. If one of the checks fails, a message similar to the following will be printed:

.. code-block:: none

    CORRUPT HEAP: Bad tail at 0x3ffe270a. Expected 0xbaad5678 got 0xbaac5678
    assertion "head != NULL" failed: file "/Users/user/esp/esp-idf/components/heap/multi_heap_poisoning.c", line 201, function: multi_heap_free
    abort() was called at PC 0x400dca43 on core 0

Consult :doc:`Heap Memory Debugging <../api-reference/system/heap_debug>` documentation for further information.

|STACK_OVERFLOW|
^^^^^^^^^^^^^^^^

.. only:: SOC_ASSIST_DEBUG_SUPPORTED

    .. _Hardware-Stack-Guard:

    Hardware Stack Guard
    """"""""""""""""""""

    {IDF_TARGET_NAME} has an integrated assist-debug module that can watch the SP register to ensure that it is within the bounds of allocated stack memory. The assist-debug module needs to set new stack bounds on every interrupt handling and FreeRTOS context switch. This can have a small impact on performance.

    Here are some additional details about the assist-debug module:

    - Implemented in hardware
    - Watches Stack Pointer register value
    - Requires no additional CPU time or memory while watching stack bounds

    When the assist-debug module detects a stack overflow, the panic handler will run and display a message that resembles the following:

    .. parsed-literal::

        Guru Meditation Error: Core 0 panic'ed (Stack protection fault).

    Hardware stack guard can be disabled using :ref:`CONFIG_ESP_SYSTEM_HW_STACK_GUARD` options.

.. _FreeRTOS-End-Of-Stack-Watchpoint:

FreeRTOS End of Stack Watchpoint
""""""""""""""""""""""""""""""""

ESP-IDF provides a custom FreeRTOS stack overflow detecting mechanism based on watchpoints. Every time FreeRTOS switches task context, one of the watchpoints is set to watch the last 32 bytes of stack.

Generally, this may cause the watchpoint to be triggered up to 28 bytes earlier than expected. The value 32 is chosen because it is larger than the stack canary size in FreeRTOS (20 bytes). Adopting this approach ensures that the watchpoint triggers before the stack canary is corrupted, not after.

.. note::

    Not every stack overflow is guaranteed to trigger the watchpoint. It is possible that the task writes to memory beyond the stack canary location, in which case the watchpoint will not be triggered.

If watchpoint triggers, the message will be similar to:

.. only:: CONFIG_IDF_TARGET_ARCH_XTENSA

    .. code-block:: none

        Debug exception reason: Stack canary watchpoint triggered (task_name)

.. only:: CONFIG_IDF_TARGET_ARCH_RISCV

    .. code-block:: none

        Guru Meditation Error: Core  0 panic'ed (Breakpoint). Exception was unhandled.

This feature can be enabled by using the :ref:`CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK` option.


FreeRTOS Stack Checks
"""""""""""""""""""""

See :ref:`CONFIG_FREERTOS_CHECK_STACKOVERFLOW`

Stack Smashing
^^^^^^^^^^^^^^

Stack smashing protection (based on GCC ``-fstack-protector*`` flags) can be enabled in ESP-IDF using :ref:`CONFIG_COMPILER_STACK_CHECK_MODE` option. If stack smashing is detected, message similar to the following will be printed:

.. code-block:: none

    Stack smashing protect failure!

    abort() was called at PC 0x400d2138 on core 0

    Backtrace: 0x4008e6c0:0x3ffc1780 0x4008e8b7:0x3ffc17a0 0x400d2138:0x3ffc17c0 0x400e79d5:0x3ffc17e0 0x400e79a7:0x3ffc1840 0x400e79df:0x3ffc18a0 0x400e2235:0x3ffc18c0 0x400e1916:0x3ffc18f0 0x400e19cd:0x3ffc1910 0x400e1a11:0x3ffc1930 0x400e1bb2:0x3ffc1950 0x400d2c44:0x3ffc1a80
    0

The backtrace should point to the function where stack smashing has occurred. Check the function code for unbounded access to local arrays.

.. only:: CONFIG_IDF_TARGET_ARCH_XTENSA

    .. |CPU_EXCEPTIONS_LIST| replace:: Illegal Instruction, Load/Store Alignment Error, Load/Store Prohibited error, Double Exception.
    .. |ILLEGAL_INSTR_MSG| replace:: IllegalInstruction
    .. |CACHE_ERR_MSG| replace:: Cache error
    .. |STACK_OVERFLOW| replace:: Stack overflow

.. only:: CONFIG_IDF_TARGET_ARCH_RISCV

    .. |CPU_EXCEPTIONS_LIST| replace:: Illegal Instruction, Load/Store Alignment Error, Load/Store Prohibited error.
    .. |ILLEGAL_INSTR_MSG| replace:: Illegal instruction
    .. |CACHE_ERR_MSG| replace:: Cache error
    .. |STACK_OVERFLOW| replace:: Stack overflow


.. only:: SOC_CPU_HAS_LOCKUP_RESET

    CPU Lockup
    ^^^^^^^^^^

    A CPU lockup reset happens when there is a double exception, i.e. when an exception occurs while the CPU is already in an exception handler. The most common cause for this is when the cache is in a state where accessing external memory becomes impossible. In such cases, the panic handler will crash as well due to being unable to fetch instructions or read data.

    To gather more information about the cause of the lockup, you can try placing the panic handler code in IRAM, which remains accessible even when the cache is disabled. This can be done with :ref:`CONFIG_ESP_PANIC_HANDLER_IRAM`.


Undefined Behavior Sanitizer (UBSAN) Checks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Undefined behavior sanitizer (UBSAN) is a compiler feature which adds run-time checks for potentially incorrect operations, such as:

- overflows (multiplication overflow, signed integer overflow)
- shift base or exponent errors (e.g., shift by more than 32 bits)
- integer conversion errors

See `GCC documentation <https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html>`_ of ``-fsanitize=undefined`` option for the complete list of supported checks.

Enabling UBSAN
""""""""""""""

UBSAN is disabled by default. It can be enabled at file, component, or project level by adding the ``-fsanitize=undefined`` compiler option in the build system.

When enabling UBSAN for code which uses the SOC hardware register header files (``soc/xxx_reg.h``), it is recommended to disable shift-base sanitizer using ``-fno-sanitize=shift-base`` option. This is due to the fact that ESP-IDF register header files currently contain patterns which cause false positives for this specific sanitizer option.

To enable UBSAN at project level, add the following code at the end of the project's ``CMakeLists.txt`` file:

.. code-block:: none

    idf_build_set_property(COMPILE_OPTIONS "-fsanitize=undefined" "-fno-sanitize=shift-base" APPEND)

Alternatively, pass these options through the ``EXTRA_CFLAGS`` and ``EXTRA_CXXFLAGS`` environment variables.

Enabling UBSAN results in significant increase of code and data size. Most applications, except for the trivial ones, will not fit into the available RAM of the microcontroller when UBSAN is enabled for the whole application. Therefore it is recommended that UBSAN is instead enabled for specific components under test.

To enable UBSAN for a specific component (``component_name``) from the project's ``CMakeLists.txt`` file, add the following code at the end of the file:

.. code-block:: none

    idf_component_get_property(lib component_name COMPONENT_LIB)
    target_compile_options(${lib} PRIVATE "-fsanitize=undefined" "-fno-sanitize=shift-base")

.. note::

    See the build system documentation for more information about :ref:`build properties <cmake-build-properties>` and :ref:`component properties <cmake-component-properties>`.

To enable UBSAN for a specific component (``component_name``) from ``CMakeLists.txt`` of the same component, add the following at the end of the file:

.. code-block:: none

    target_compile_options(${COMPONENT_LIB} PRIVATE "-fsanitize=undefined" "-fno-sanitize=shift-base")

UBSAN Output
""""""""""""

When UBSAN detects an error, a message and the backtrace are printed, for example:

.. code-block:: none

    Undefined behavior of type out_of_bounds

    Backtrace:0x4008b383:0x3ffcd8b0 0x4008c791:0x3ffcd8d0 0x4008c587:0x3ffcd8f0 0x4008c6be:0x3ffcd950 0x400db74f:0x3ffcd970 0x400db99c:0x3ffcd9a0

When using :doc:`IDF Monitor <tools/idf-monitor>`, the backtrace will be decoded to function names and source code locations, pointing to the location where the issue has happened (here it is ``main.c:128``):

.. code-block:: none

    0x4008b383: panic_abort at /path/to/esp-idf/components/esp_system/panic.c:367

    0x4008c791: esp_system_abort at /path/to/esp-idf/components/esp_system/system_api.c:106

    0x4008c587: __ubsan_default_handler at /path/to/esp-idf/components/esp_system/ubsan.c:152

    0x4008c6be: __ubsan_handle_out_of_bounds at /path/to/esp-idf/components/esp_system/ubsan.c:223

    0x400db74f: test_ub at main.c:128

    0x400db99c: app_main at main.c:56 (discriminator 1)

The types of errors reported by UBSAN can be as follows:


.. list-table::
    :widths: 40 60
    :header-rows: 1

    * - Name
      - Meaning
    * - ``type_mismatch``, ``type_mismatch_v1``
      - Incorrect pointer value: null, unaligned, not compatible with the given type.
    * - ``add_overflow``, ``sub_overflow``, ``mul_overflow``, ``negate_overflow``
      - Integer overflow during addition, subtraction, multiplication, negation.
    * - ``divrem_overflow``
      - Integer division by 0 or ``INT_MIN``.
    * - ``shift_out_of_bounds``
      - Overflow in left or right shift operators.
    * - ``out_of_bounds``
      - Access outside of bounds of an array.
    * - ``unreachable``
      - Unreachable code executed.
    * - ``missing_return``
      - Non-void function has reached its end without returning a value (C++ only).
    * - ``vla_bound_not_positive``
      - Size of variable length array is not positive.
    * - ``load_invalid_value``
      - Value of ``bool`` or ``enum`` (C++ only) variable is invalid (out of bounds).
    * - ``nonnull_arg``
      - Null argument passed to a function which is declared with a ``nonnull`` attribute.
    * - ``nonnull_return``
      - Null value returned from a function which is declared with ``returns_nonnull`` attribute.
    * - ``builtin_unreachable``
      - ``__builtin_unreachable`` function called.
    * - ``pointer_overflow``
      - Overflow in pointer arithmetic.
````

## File: docs/en/api-guides/file-system-considerations.rst
````
File System Considerations
==========================

:link_to_translation:`zh_CN:[中文]`

This chapter is intended to help you decide which file system is most suitable for your application. It points out specific features and properties of the file systems supported by the ESP-IDF, which are important in typical use-cases rather than describing all the specifics or comparing implementation details. Technical details for each file system are available in their corresponding documentation.

Currently, the ESP-IDF framework supports three file systems. ESP-IDF provides convenient APIs to handle the mounting and dismounting of file systems in a unified way. File and directory access is implemented via C/POSIX standard file APIs, allowing all applications to use the same interface regardless of the underlying file system:

- :ref:`FatFS <fatfs-fs-section>`
- :ref:`SPIFFS <spiffs-fs-section>`
- :ref:`LittleFS <littlefs-fs-section>`

All of them are based on 3rd-party libraries connected to the ESP-IDF through various wrappers and modifications.

ESP-IDF also provides the NVS Library API for simple data storage use cases, using keys to access associated values. While it is not a full-featured file system, it is a good choice for storing configuration data, calibration data, and similar information. For more details, see the :ref:`NVS Library <nvs-fs-section>` section.

The most significant properties and features of above-mentioned file systems are summarized in the following table:

.. list-table::
    :widths: 20 40 40 40
    :header-rows: 1

    * -
      - FatFS
      - SPIFFS
      - LittleFS
    * - Features
      - * Implements MS FAT12, FAT16, FAT32 and optionally exFAT variants
        * General purpose filesystem, widely compatible across most HW platforms
        * Well documented
        * Thread safe
      - * Developed for NOR flash devices on embedded systems, low RAM usage
        * Implements static wear levelling
        * Limited documentation, no ongoing development
        * Thread safe
      - * Designed as fail-safe, with own wear levelling and with fixed amount of RAM usage independent on the file system size
        * Well documented
        * Thread safe
    * - Storage units and limits
      - * Clusters (1–128 sectors)
        * Supported sector sizes: 512 B, 4096 B
        * FAT12: cluster size 512 B – 8 kB, max 4085 clusters
        * FAT16: cluster size 512 B – 64 kB, max 65525 clusters
        * FAT32: cluster size 512 B – 32 kB, max 268435455 clusters
      - * Logical pages, logical blocks (consists of pages)
        * Typical setup: page = 256 B, block = 64 kB
      - * Blocks, metadata pairs
        * Typical block size: 4 kB
    * - Wear Levelling
      - Optional (for SPI Flash)
      - Integrated
      - Integrated
    * - Minimum partition size
      - * 8 sectors with wear levelling on (4 FATFS sectors + 4 WL sectors with WL sector size = 4096 B)
        * plus 4 sectors at least
        * real number given by WL configuration (Safe, Perf)
      - * 6 logical blocks
        * 8 pages per block
      - Not specified, theoretically 2 blocks
    * - Maximum partition size
      - * FAT12: approx. 32 MB with 8 kB clusters
        * FAT16: approx. 4 GB with 64 kB clusters (theoretical)
        * FAT32: approx. 8 TB with 32 kB clusters (theoretical)
      - Absolute maximum not specified, more than 1024 pages per block not recommended
      - Not specified, theoretically around 2 GB
    * - Directory Support
      - * Yes (max 65536 entries in a common FAT directory)
        * Limitations:
            * FAT12: max 224 files in the Root directory
            * FAT16: max 512 files in the Root directory
            * FAT32: the Root is just another directory
      - No
      - Yes
    * - Power failure protection
      - No
      - Partial (see :ref:`spiffs-fs-section`)
      - Yes (integrated)
    * - Encryption support
      - Yes
      - No
      - Yes
    * - Supported targets
      - * SPI Flash (NOR)
        * SD cards
      - SPI Flash (NOR)
      - * SPI Flash (NOR)
        * SD cards (IDF >= v5.0)

For file systems performance comparison using various configurations and parameters, see Storage performance benchmark example :example:`storage/perf_benchmark`.


.. _fatfs-fs-section:

FatFS
---------

The most supported file system, recommended for common applications - file/directory operations, data storage, logging, etc. It provides automatic resolution of specific FAT system type and is widely compatible with PC or other platforms. FatFS supports partition encryption, read-only mode, optional wear-levelling for SPI Flash (SD cards use own built-in WL), equipped with auxiliary host side tools (generators and parsers, Python scripts). It supports SDMMC access. The biggest weakness is its low resilience against sudden power-off events. To mitigate such a scenario impact, the ESP-IDF FatFS default setup deploys 2 FAT table copies. This option can be disabled by setting :cpp:member:`esp_vfs_fat_mount_config_t::use_one_fat` flag (the 2-FAT processing is fully handled by the FatFS library). See also related examples.

**Related documents:**

- `FatFS source site <http://elm-chan.org/fsw/ff/>`_
- More about `FAT table size limits <https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#Size_limits>`_
- :ref:`Using FatFS with VFS <using-fatfs-with-vfs>`
- :ref:`Using FatFS with VFS and SD cards <using-fatfs-with-vfs-and-sdcards>`
- ESP-IDF FatFS tools: :ref:`Partition generator <fatfs-partition-generator>` and :ref:`Partition analyzer <fatfs-partition-analyzer>`

**Examples:**

* :example:`storage/sd_card` demonstrates how to access the SD card that uses the FAT file system.
* :example:`storage/fatfs/ext_flash` demonstrates how to access the external flash that uses the FAT file system.


.. _spiffs-fs-section:

SPIFFS
----------------------

SPIFFS is a file system providing certain level of power-off safety (see repair-after-restart function :cpp:func:`esp_spiffs_check`) and built-in wear levelling. It tends to slow down when exceeding around 70% of the dedicated partition size due to its garbage collector implementation, and also doesn't support directories. It is useful for applications depending only on few files (possibly large) and requiring high level of consistency. Generally, the SPIFFS needs less RAM resources than FatFS and supports flash chips up to 128 MB in size. Please keep in mind the SPIFFS is not being developed and maintained anymore, so consider precisely whether its advantages for your project really prevail over the other file systems.

**Related documents:**

* :doc:`SPIFFS Filesystem <../api-reference/storage/spiffs>`
* :ref:`Tools For Generating SPIFFS Images <spiffs-generator>`

**Examples:**

* :example:`storage/spiffs` demonstrates how to use SPIFFS on {IDF_TARGET_NAME} chip.


.. _littlefs-fs-section:

LittleFS
----------------------

LittleFS is a block based file system designed for microcontrollers and embedded devices. It provides a good level of power failure resilience, implements dynamic wear levelling, and has very low RAM requirements. The system also has configurable limits and integrated SD/MMC card support. It is a recommended choice for general type of application. The only disadvantage is the file system not being natively compatible with other platforms (unlike FatFS).

LittleFS is available as external component in the `ESP Component Registry <https://components.espressif.com/>`_. See `LittleFS component page <https://components.espressif.com/components/joltwallet/littlefs>`_ for the details on including the file system into your project.

**Related documents:**

* `LittleFS project home (sources, documentation) <https://github.com/littlefs-project/littlefs>`_
* `LittleFS auxiliary tools and related projects <https://github.com/littlefs-project/littlefs?tab=readme-ov-file#related-projects>`_
* `LittleFS port for ESP-IDF <https://github.com/joltwallet/esp_littlefs>`_
* `ESP-IDF LittleFS component <https://components.espressif.com/components/joltwallet/littlefs>`_

**Examples:**

* :example:`storage/littlefs` demonstrates how to use LittleFS on {IDF_TARGET_NAME} chip.

.. _nvs-fs-section:

NVS Library
---------------

Non-volatile Storage (NVS) is useful for applications depending on handling numerous key-value pairs, for instance application system configuration. For convenience, the key space is divided into namespaces, each namespace is a separate storage area. Besides the basic data types up to the size of 64-bit integers, the NVS also supports zero terminated strings and blobs - binary data of arbitrary length.

Features include:

* Flash wear leveling by design.
* Sudden power-loss protection (data is stored in a way that ensures atomic updates).
* Encryption support (AES-XTS).
* Tooling is provided for both data preparation during manufacturing and offline analysis.

Points to keep in mind when developing NVS related code:

* The recommended use case is storing configuration data that does not change frequently.
* NVS is not suitable for logging or other use cases with frequent, large data updates. NVS works best with small updates and low-frequency writes. Another limitation is the maximum number of flash page erase cycles, which is typically around 100,000 for NOR flash devices.
* If the application needs to store groups of data with significantly different update rates, it is recommended to use separate NVS flash partitions for each group. This makes wear leveling easier to manage and reduces the risk of data corruption.
* The default NVS partition (the one labeled "nvs") is used by other ESP-IDF components such as WiFi, Bluetooth, etc. It is recommended to use a separate partition for application data to avoid conflicts with other components.
* The allocation unit for NVS storage in flash memory is one page—4,096 bytes. At least three pages are needed for each NVS partition to function properly. One page is always reserved and never used for data storage.
* Before writing or updating existing data, there must be enough free space in the NVS partition to store both the old and new data. The NVS library doesn't support partial updates. This can be especially challenging with large BLOBs spanning flash page boundaries, resulting in longer write times and increased overhead space consumption.
* The NVS library cannot ensure data consistency in out-of-spec power environments, such as systems powered by batteries or solar panels. Misinterpretation of flash data in such situations can lead to corruption of the NVS flash partition. Developers should include data recovery code, e.g., based on a read-only data partition with factory settings.
* An initialized NVS library leaves a RAM footprint, which scales linearly with the overall size of the flash partitions and the number of cached keys.

**Read-only NVS partitions:**

* Read-only partitions can be used to store data that should not be modified at runtime. This is useful for storing firmware or configuration data that should not be changed by the application.
* NVS partitions can be flagged as ``readonly`` in the partition table CSV file. Size of read-only NVS partition can be as small as one page (4 KiB/``0x1000``), which is not possible for standard read-write NVS partitions.
* Partitions of sizes ``0x1000`` and ``0x2000`` are always read-only and partitions of size ``0x3000`` and above are always read-write capable (still can be opened in read-only mode in the code).

**Related documents:**

- To learn more about the API and NVS library details, see the :doc:`NVS documentation page <../api-reference/storage/nvs_flash>`.
- For mass production, you can use the :doc:`NVS Partition Generator Utility <../api-reference/storage/nvs_partition_gen>`.
- For offline NVS partition analysis, you can use the :doc:`NVS Partition Parser Utility <../api-reference/storage/nvs_partition_parse>`.
- For more information about read-only NVS partitions, see the :ref:`Read-only NVS <read-only-nvs>`.

**Examples:**

- :example:`storage/nvs/nvs_rw_value` demonstrates how to use NVS to write and read a single integer value.
- :example:`storage/nvs/nvs_rw_blob` demonstrates how to use NVS to write and read a blob.
- :example:`security/nvs_encryption_hmac` demonstrates NVS encryption using the HMAC peripheral, where the encryption keys are derived from the HMAC key burnt in eFuse.
- :example:`security/flash_encryption` demonstrates the flash encryption workflow including NVS partition creation and usage.


File handling design considerations
-----------------------------------

Here are several recommendation for building reliable storage features into your application:

* Use C Standard Library file APIs (ISO or POSIX) wherever possible. This high-level interface guarantees you will not need to change much, if it comes for instance to switching to a different file system. All the ESP-IDF supported file systems work as underlying layer for C STDLIB calls, so the specific file system details are nearly transparent to the application code. The only parts unique to each single system are formatting, mounting and diagnostic/repair functions
* Keep the file system dependent code separated, use wrappers to allow minimum change updates
* Design reasonable structure of your application file storage:
    * Distribute the load evenly, if possible. Use meaningful number of directories/subdirectories (for instance FAT12 can keep only 224 records in its root directory).
    * Avoid using too many files or too large files (though the latter usually causes less troubles than the former). Each file equals to a record in the system's internal "database", which can easily end up in the necessary overhead consuming more space than the data stored. Even worse case is exhausting the filesystem's resources and subsequent failure of the application - which can happen really quickly in embedded systems' environment.
    * Be cautious about number of write or erase operations performed in SPI Flash memory (for example, each write in the FatFS involves full erase of the area to be written). NOR Flash devices typically survive 100,000+ erase cycles per sector, and their lifetime is extended by the Wear-Levelling mechanism (implemented as a standalone component in corresponding driver stack, transparent from the application's perspective). The Wear-Levelling algorithm rotates the Flash memory sectors all around given partition space, so it requires some disk space available for the virtual sector shuffle. If you create "well-tailored" partition with the minimum space needed and manage to fill it with your application data, the Wear Levelling becomes ineffective and your device would degrade quickly. Projects with Flash write frequency around 500ms are fully capable to destroy average ESP32 flash in few days time (real world example).
    * With the previous point given, consider using reasonably large partitions to ensure safe margins for your data. It is usually cheaper to invest into extra Flash space than to forcibly resolve troubles unexpectedly happening in the field.
    * Think twice before deciding for specific file system - they are not 100% equal and each application has own strategy and requirements. For instance, the NVS is not suitable for storing a production data, as its design doesn't deal well with too many items being stored (recommended maximum for NVS partition size would be around 128 kB).


Encrypting partitions
---------------------

{IDF_TARGET_NAME} based chips provide several features to encrypt the contents of various partitions within chip's main SPI flash memory. All the necessary information can be found in chapters :doc:`Flash Encryption <../security/flash-encryption>` and :doc:`NVS Encryption <../api-reference/storage/nvs_encryption>`. Both variants use the AES family of algorithms, the Flash Encryption provides hardware-driven encryption scheme and is transparent from the software's perspective, whilst the NVS Encryption is a software feature implemented using mbedTLS component (though the mbedTLS can internally use the AES hardware accelerator, if available on given chip model). The latter requires the Flash Encryption enabled as the NVS Encryption needs a proprietary encrypted partition to hold its keys, and the NVS internal structure is not compatible with the Flash Encryption design. Therefore, both features come separate.

Considering the storage security scheme and the design of {IDF_TARGET_NAME} chips, there are several implications that may not be fully obvious in the main documents:

* The Flash encryption applies only to the main SPI Flash memory, due to its cache module design (all the "transparent" encryption APIs run over this cache). This implies that external flash partitions cannot be encrypted using the native Flash Encryption means.
* External partition encryption can be deployed by implementing custom encrypt/decrypt code in appropriate driver APIs - either by implementing own SPI flash driver (see :example:`storage/custom_flash_driver`) or by customizing higher levels in the driver stack, for instance by providing own :ref:`FatFS disk IO layer <fatfs-diskio-layer>`.
````

## File: docs/en/api-guides/flash_psram_config.rst
````
SPI Flash and External SPI RAM Configuration
============================================

:link_to_translation:`zh_CN:[中文]`

This page is a guide for configuring SPI flash and external SPI RAM. Supported frequency and mode combination, error handling are also elaborated.

Terminology
-----------

.. list-table::
    :header-rows: 1
    :widths: 20 80
    :align: center

    * - Term
      - Definition
    * - **SPI**
      - Serial Peripheral Interface
    * - **MSPI**
      - Memory SPI Peripheral, SPI Peripheral dedicated for memory
    * - **SDR**
      - Single Data Transfer Rate (SDR), also known as STR (Single Transfer Rate)
    * - **DDR**
      - Double Data Transfer Rate (DDR), also known as DTR (Double Transfer Rate)
    * - **line mode**
      - Number of signals used to transfer data in the data phase of SPI transactions. e.g., for 4-bit-mode, the speed of the data phase would be 4 bit per clock cycle.
    * - **FxRx**
      - F stands for flash, R stands for PSRAM, x stands for line mode. e.g., F4R4 stands for an {IDF_TARGET_NAME} with Quad flash and Quad PSRAM.

.. note::

    On {IDF_TARGET_NAME}, MSPI stands for the SPI0/1. SPI0 and SPI1 share a common SPI bus. The main flash and PSRAM are connected to the MSPI peripheral. CPU accesses them via Cache.


.. _flash-psram-configuration:

How to Configure Flash and PSRAM
--------------------------------

``idf.py menuconfig`` is used to open the configuration menu.

Configure the Flash
^^^^^^^^^^^^^^^^^^^

The flash related configurations are under ``Serial flasher config`` menu.

1. Flash type used on the board. For Octal flash, select :ref:`CONFIG_ESPTOOLPY_OCT_FLASH`. For Quad flash, uncheck this configuration.
2. Flash line mode. Select a line mode in :ref:`CONFIG_ESPTOOLPY_FLASHMODE`. The higher the line mode is, the faster the SPI speed is. See terminology above about the line mode.
3. Flash sample mode. Select a sample mode in :ref:`CONFIG_ESPTOOLPY_FLASH_SAMPLE_MODE`. DDR mode is faster than SDR mode. See terminology above about SDR and DDR mode.
4. Flash speed. Select a Flash frequency in :ref:`CONFIG_ESPTOOLPY_FLASHFREQ`.
5. Flash size. Flash size, in megabytes. Select a flash size in :ref:`CONFIG_ESPTOOLPY_FLASHSIZE`.

Configure the PSRAM
^^^^^^^^^^^^^^^^^^^

To enable PSRAM, please enable the :ref:`CONFIG_SPIRAM` under ``Component config / Hardware Settings`` menu. Then all the PSRAM related configurations will be visible under ``SPI RAM config`` menu.

1. PSRAM type used on the board. Select a type in :ref:`CONFIG_SPIRAM_MODE` for Quad or Octal PSRAM.
2. PSRAM speed. Select a PSRAM frequency in :ref:`CONFIG_SPIRAM_SPEED`.

.. note::

    Configuration 1 of flash and PSRAM should be selected according to your actual hardware.

    For the reset of the above configurations:

    - Flash and PSRAM share the same internal clock.
    - Quad flash only supports STR mode. Octal flash may support either/both STR/DTR modes under OPI mode, depending on the flash model and the vendor.
    - Quad PSRAM only supports STR mode, while Octal PSRAM only supports DTR mode.

    Therefore, some limitations should be noticed when configuring configuration 2, 3 and 4 of flash, and configuration 2 of PSRAM. Please refer to :ref:`All Supported Modes and Speeds <flash-psram-combination>`.

.. note::

    If a board with Octal flash resets before the second stage bootloader, please refer to :ref:`Error Handling Chapter <flash-psram-error>`.


.. _flash-psram-combination:

All Supported Modes and Speeds
------------------------------

.. note::

    For MSPI DDR mode, the data are sampled on both the positive edge and the negative edge. e.g., if a flash is set to 80 MHz and DDR mode, then the final speed of the flash is 160 MHz. This is faster than the flash setting to 120 Mhz and STR mode.

.. important::

    120 MHz DDR mode is an experimental feature. You will only see it when:

    - :ref:`CONFIG_IDF_EXPERIMENTAL_FEATURES` is enabled

    With above step, you will find 120 MHz option is visible.

    Risks:

    If your chip powers on at a certain temperature, then after the temperature increases or decreases over 20 celsius degree, the accesses to/from PSRAM/flash will crash randomly. Flash access crash will lead to program crash.

    Note 20 celsius degree is not a totally correct number. This value may changes among chips.

.. note::

    The PSRAM requires a phase point calibration algorithm when operating at 120M. The phase point setting is related to the temperature at startup. When the temperature increases / decreases significantly during the operation of the chip, the PSRAM may experience read/write errors. To solve this problem, you can enable dynamic adjustment of the PSRAM phase point based on the temperature value with :ref:`CONFIG_SPIRAM_TIMING_TUNING_POINT_VIA_TEMPERATURE_SENSOR`. This creates a task that measures the temperature every :ref:`CONFIG_SPIRAM_TIMING_MEASURE_TEMPERATURE_INTERVAL_SECOND` seconds and adjusts the PSRAM phase point accordingly.

F8R8 Hardware
^^^^^^^^^^^^^

.. list-table::
    :header-rows: 1
    :widths: 20 30 20 30
    :align: center

    * - Group
      - Flash mode
      - Group
      - PSRAM mode
    * - A
      - 120 MHz DDR
      - A
      - 120 MHz DDR
    * - A
      - 120 MHz SDR
      - A
      -
    * - B
      - 80 MHz DDR
      - B
      - 80 MHz DDR
    * - C
      - 80 MHz SDR
      - C
      - 40 MHz DDR
    * - C
      - 40 MHz DDR
      - C
      -
    * - C
      - < 40 MHz
      - C
      -
    * - D
      -
      - D
      - disable

1. Flash mode in group A works with PSRAM mode in group A/D.
2. Flash mode in group B/C works with PSRAM mode in group B/C/D.


F4R8 Hardware
^^^^^^^^^^^^^

.. list-table::
    :header-rows: 1
    :widths: 20 30 20 30
    :align: center

    * - Group
      - Flash mode
      - Group
      - PSRAM mode
    * - A
      - 120 MHz SDR
      - A
      - 120 MHz DDR
    * - B
      - 80 MHz SDR
      - B
      - 80 MHz DDR
    * - C
      - 40 MHz SDR
      - C
      - 40 MHz DDR
    * - C
      - 20 MHz SDR
      - C
      -
    * - D
      -
      - D
      - disable

1. Flash mode in group A works with PSRAM mode in group A/D.
2. Flash mode in group B/C works with PSRAM mode in group B/C/D.


F4R4 Hardware
^^^^^^^^^^^^^

.. list-table::
    :header-rows: 1
    :widths: 20 30 20 30
    :align: center

    * - Group
      - Flash mode
      - Group
      - PSRAM mode
    * - A
      - 120 MHz
      - A
      - 120 MHz
    * - B
      - 80 MHz
      - B
      - 80 MHz
    * - C
      - 40 MHz
      - C
      - 40 MHz
    * - C
      - 20 MHz
      - C
      -
    * - D
      -
      - D
      - disable

1. Flash mode in group A works with PSRAM in group A/C/D.
2. Flash mode in group B works with PSRAM in group B/C/D.
3. Flash mode in group C works with PSRAM in group A/B/C/D.


.. _flash-psram-error:

Error Handling
--------------

1. If a board with Octal flash resets before the second stage bootloader:

    .. code-block:: c

        ESP-ROM:esp32s3-20210327
        Build:Mar 27 2021
        rst:0x7 (TG0WDT_SYS_RST),boot:0x18 (SPI_FAST_FLASH_BOOT)
        Saved PC:0x400454d5
        SPIWP:0xee
        mode:DOUT, clock div:1
        load:0x3fcd0108,len:0x171c
        ets_loader.c 78

   this may mean that the necessary eFuses are not correctly burnt. Please check the eFuse bits of the chip using ``idf.py efuse-summary``.

   The first stage (ROM) bootloader relies on an eFuse bit ``FLASH_TYPE`` to reset the flash into the default mode (SPI mode). If this bit is not burnt and the flash is working in OPI mode, the first stage (ROM) bootloader may not be able to read from the flash and load the following images.

2. If you enabled :ref:`CONFIG_ESPTOOLPY_OCT_FLASH`, and there's an error log saying:

    .. code-block:: c

        Octal flash option selected, but EFUSE not configured!

   this means:

   - either you're using a board with a Quad flash,
   - or you're using a board with an Octal flash, but the eFuse bit ``FLASH_TYPE`` isn't burnt. Espressif guarantees this bit is burnt during module manufacturing, but if the module is manufactured by others, this may happen.


Here is a method to burn the eFuse bit:

.. code-block:: shell

    idf.py -p PORT efuse-burn --do-not-confirm FLASH_TYPE 1

.. note::

    This step is irreversible. Please do check if your hardware is actually using an Octal flash.
````

## File: docs/en/api-guides/general-notes.rst
````
:orphan:

General Notes About ESP-IDF Programming
=======================================

:link_to_translation:`zh_CN:[中文]`

This page has been split into two new pages:

- :doc:`startup`
- :doc:`memory-types`
````

## File: docs/en/api-guides/hardware-abstraction.rst
````
Hardware Abstraction
====================

:link_to_translation:`zh_CN:[中文]`

ESP-IDF provides a group of APIs for hardware abstraction. These APIs allow you to control peripherals at different levels of abstraction, giving you more flexibility compared to using only the ESP-IDF drivers to interact with hardware. ESP-IDF Hardware abstraction is likely to be useful for writing high-performance bare-metal drivers, or for attempting to port an ESP chip to another platform.

This guide is split into the following sections:

    1. :ref:`hw-abstraction-architecture`
    2. :ref:`hw-abstraction-ll-layer`
    3. :ref:`hw-abstraction-hal-layer`

.. warning::

    Hardware abstraction API (excluding the driver and ``xxx_types.h``) should be considered an experimental feature, thus cannot be considered public API. The hardware abstraction API does not adhere to the API name changing restrictions of ESP-IDF's versioning scheme. In other words, it is possible that Hardware Abstraction API may change in between non-major release versions.

.. note::

    Although this document mainly focuses on hardware abstraction of peripherals, e.g., UART, SPI, I2C, certain layers of hardware abstraction extend to other aspects of hardware as well, e.g., some of the CPU's features are partially abstracted.

.. _hw-abstraction-architecture:

Architecture
------------

Hardware abstraction in ESP-IDF is comprised of the following layers, ordered from low level of abstraction that is closer to hardware, to high level of abstraction that is further away from hardware.

- Low Level (LL) Layer
- Hardware Abstraction Layer (HAL)
- Driver Layers

Each hardware module has its own independent ``esp_hal_xxx`` component, which contains both the LL and the HAL of that module. Each layer is dependent on the layer below it, i.e, driver depends on HAL, HAL depends on LL, LL depends on the register header files.

For a particular peripheral ``xxx``, its hardware abstraction generally consists of the header files described in the table below. Files that are **Target Specific** have a separate implementation for each target, i.e., a separate copy for each chip. However, the ``#include`` directive is still target-independent, i.e., is the same for different targets, as the build system automatically includes the correct version of the header and source files.

.. |br| raw:: html

    <br>

.. list-table:: Hardware Abstraction Header Files
    :widths: 25 5 70
    :header-rows: 1

    * - Include |br| Directive
      - Target |br| Specific
      - Description
    * - ``#include 'soc/xxx_caps.h"``
      - Y
      - This header contains a list of C macros specifying the various capabilities of the {IDF_TARGET_NAME}'s peripheral ``xxx``. Hardware capabilities of a peripheral include things such as the number of channels, DMA support, hardware FIFO/buffer lengths, etc.
    * - ``#include "soc/xxx_struct.h"`` |br| ``#include "soc/xxx_reg.h"``
      - Y
      - The two headers contain a representation of a peripheral's registers in C structure and C macro format respectively, allowing you to operate a peripheral at the register level via either of these two header files.
    * - ``#include "soc/xxx_pins.h"``
      - Y
      - If certain signals of a peripheral are mapped to a particular pin of the {IDF_TARGET_NAME}, their mappings are defined in this header as C macros.
    * - ``#include "soc/xxx_periph.h"``
      - N
      - This header is mainly used as a convenience header file to automatically include ``xxx_caps.h``, ``xxx_struct.h``, and ``xxx_reg.h``.
    * - ``#include "hal/xxx_types.h``
      - N
      - This header contains type definitions and macros that are shared among the LL, HAL, and driver layers. Moreover, it is considered public API thus can be included by the application level. The shared types and definitions usually related to non-implementation specific concepts such as the following:

          - Protocol-related types/macros such a frames, modes, common bus speeds, etc.
          - Features/characteristics of an ``xxx`` peripheral that are likely to be present on any implementation (implementation-independent) such as channels, operating modes, signal amplification or attenuation intensities, etc.
    * - ``#include "hal/xxx_ll.h"``
      - Y
      - This header contains the Low Level (LL) Layer of hardware abstraction. LL Layer API are primarily used to abstract away register operations into readable functions.
    * - ``#include "hal/xxx_hal.h"``
      - Y
      - The Hardware Abstraction Layer (HAL) is used to abstract away peripheral operation steps into functions (e.g., reading a buffer, starting a transmission, handling an event, etc). The HAL is built on top of the LL Layer.
    * - ``#include "driver/xxx.h"``
      - N
      - The driver layer is the highest level of ESP-IDF's hardware abstraction. Driver layer API are meant to be called from ESP-IDF applications, and internally utilize OS primitives. Thus, driver layer API are event-driven, and can used in a multi-threaded environment.


.. _hw-abstraction-ll-layer:

LL (Low Level) Layer
--------------------

The primary purpose of the LL Layer is to abstract away register field access into more easily understandable functions. LL functions essentially translate various in/out arguments into the register fields of a peripheral in the form of get/set functions. All the necessary bit shifting, masking, offsetting, and endianness of the register fields should be handled by the LL functions.

.. code-block:: c

    //Inside xxx_ll.h

    static inline void xxx_ll_set_baud_rate(xxx_dev_t *hw,
                                            xxx_ll_clk_src_t clock_source,
                                            uint32_t baud_rate) {
        uint32_t src_clk_freq = (source_clk == XXX_SCLK_APB) ? APB_CLK_FREQ : REF_CLK_FREQ;
        uint32_t clock_divider = src_clk_freq / baud;
        // Set clock select field
        hw->clk_div_reg.divider = clock_divider >> 4;
        // Set clock divider field
        hw->config.clk_sel = (source_clk == XXX_SCLK_APB) ? 0 : 1;
    }

    static inline uint32_t xxx_ll_get_rx_byte_count(xxx_dev_t *hw) {
        return hw->status_reg.rx_cnt;
    }

The code snippet above illustrates typical LL functions for a peripheral ``xxx``. LL functions typically have the following characteristics:

- All LL functions are defined as ``static inline`` so that there is minimal overhead when calling these functions due to compiler optimization. These functions are not guaranteed to be inlined by the compiler, so any LL function that is called when the cache is disabled (e.g., from an IRAM ISR context) should be marked with ``__attribute__((always_inline))``.
- The first argument should be a pointer to a ``xxx_dev_t`` type. The ``xxx_dev_t`` type is a structure representing the peripheral's registers, thus the first argument is always a pointer to the starting address of the peripheral's registers. Note that in some cases where the peripheral has multiple channels with identical register layouts, ``xxx_dev_t *hw`` may point to the registers of a particular channel instead.
- LL functions should be short, and in most cases are deterministic. In other words, in the worst case, runtime of the LL function can be determined at compile time. Thus, any loops in LL functions should be finite bounded; however, there are currently a few exceptions to this rule.
- LL functions are not thread-safe, it is the responsibility of the upper layers (driver layer) to ensure that registers or register fields are not accessed concurrently.


.. _hw-abstraction-hal-layer:

HAL (Hardware Abstraction Layer)
--------------------------------

The HAL layer models the operational process of a peripheral as a set of general steps, where each step has an associated function. For each step, the details of a peripheral's register implementation (i.e., which registers need to be set/read) are hidden (abstracted away) by the HAL. By modeling peripheral operation as a set of functional steps, any minor hardware implementation differences of the peripheral between different targets or chip versions can be abstracted away by the HAL (i.e., handled transparently). In other words, the HAL API for a particular peripheral remains mostly the same across multiple targets/chip versions.

The following HAL function examples are selected from the Watchdog Timer HAL as each function maps to one of the steps in a WDT's operation life cycle, thus illustrating how a HAL abstracts a peripheral's operation into functional steps.

.. code-block:: c

    // Initialize one of the WDTs
    void wdt_hal_init(wdt_hal_context_t *hal, wdt_inst_t wdt_inst, uint32_t prescaler, bool enable_intr);

    // Configure a particular timeout stage of the WDT
    void wdt_hal_config_stage(wdt_hal_context_t *hal, wdt_stage_t stage, uint32_t timeout, wdt_stage_action_t behavior);

    // Start the WDT
    void wdt_hal_enable(wdt_hal_context_t *hal);

    // Feed (i.e., reset) the WDT
    void wdt_hal_feed(wdt_hal_context_t *hal);

    // Handle a WDT timeout
    void wdt_hal_handle_intr(wdt_hal_context_t *hal);

    // Stop the WDT
    void wdt_hal_disable(wdt_hal_context_t *hal);

    // De-initialize the WDT
    void wdt_hal_deinit(wdt_hal_context_t *hal);

.. _hw-abstraction-hal-layer-disable-rtc-wdt:

To Disable RTC_WDT
^^^^^^^^^^^^^^^^^^

.. code-block:: c

    wdt_hal_context_t rtc_wdt_ctx = RWDT_HAL_CONTEXT_DEFAULT();
    wdt_hal_write_protect_disable(&rtc_wdt_ctx);
    wdt_hal_disable(&rtc_wdt_ctx);
    wdt_hal_write_protect_enable(&rtc_wdt_ctx);

.. _hw-abstraction-hal-layer-feed-rtc-wdt:

To Reset the RTC_WDT Counter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    wdt_hal_context_t rtc_wdt_ctx = RWDT_HAL_CONTEXT_DEFAULT();
    wdt_hal_write_protect_disable(&rtc_wdt_ctx);
    wdt_hal_feed(&rtc_wdt_ctx);
    wdt_hal_write_protect_enable(&rtc_wdt_ctx);

HAL functions generally have the following characteristics:

- The first argument to a HAL function has the ``xxx_hal_context_t *`` type. The HAL context type is used to store information about a particular instance of the peripheral (i.e., the context instance). A HAL context is initialized by the ``xxx_hal_init()`` function and can store information such as the following:

    - The channel number of this instance
    - Pointer to the peripheral's (or channel's) registers  (i.e., a ``xxx_dev_t *`` type)
    - Information about an ongoing transaction (e.g., pointer to DMA descriptor list in use)
    - Some configuration values for the instance (e.g., channel configurations)
    - Variables to maintain state information regarding the instance (e.g., a flag to indicate if the instance is waiting for transaction to complete)

- HAL functions should not contain any OS primitives such as queues, semaphores, mutexes, etc. All synchronization/concurrency should be handled at higher layers (e.g., the driver).
- Some peripherals may have steps that cannot be further abstracted by the HAL, thus end up being a direct wrapper (or macro) for an LL function.
- Some HAL functions may be placed in IRAM thus may carry an ``IRAM_ATTR`` or be placed in a separate ``xxx_hal_iram.c`` source file.
````

## File: docs/en/api-guides/hlinterrupts.rst
````
High Priority Interrupts
========================

:link_to_translation:`zh_CN:[中文]`

.. toctree::
   :maxdepth: 1

The Xtensa architecture supports 32 interrupts, divided over 7 priority levels from level 1 to 7, with level 7 being an non-maskable interrupt (NMI), plus an assortment of exceptions. On the {IDF_TARGET_NAME}, the :doc:`../api-reference/system/intr_alloc` can route most interrupt sources to these interrupts via the interrupt mux. Normally, interrupts are written in C, but ESP-IDF allows high-priority interrupts to be written in assembly as well, resulting in very low interrupt latencies.

Interrupt Priorities
--------------------

.. only:: esp32

  .. list-table::
      :header-rows: 1
      :widths: 20 30 50
      :align: center

      * - Priority Level
        - Symbol
        - Remark
      * - 1
        - N/A
        - Exception and low priority interrupts, handled by ESP-IDF.
      * - 2-3
        - N/A
        - Medium priority interrupts, handled by ESP-IDF.
      * - 4
        - xt_highint4
        - High priority interrupt, free to use. [1]_
      * - 5
        - xt_highint5
        - Normally used by ESP-IDF debug logic. [1]_
      * - NMI
        - xt_nmi
        - Non-maskable interrupt, free to use.
      * - dbg
        - xt_debugexception
        - Debug exception. Called on e.g., a BREAK instruction. [2]_

  .. [1] ESP-IDF debug logic can be configured to run on ``xt_highint4`` or ``xt_highint5`` in :ref:`CONFIG_ESP_SYSTEM_CHECK_INT_LEVEL`. Bluetooth's interrupt can be configured to run on priority level 4 by enabling :ref:`CONFIG_BTDM_CTRL_HLI`. If :ref:`CONFIG_BTDM_CTRL_HLI` is enabled, ESP-IDF debug logic must be running on priority level 5 interrupt.
  .. [2] If :ref:`CONFIG_BTDM_CTRL_HLI` is enabled, ``xt_debugexception`` is used to fix the `live lock issue <https://www.espressif.com/sites/default/files/documentation/eco_and_workarounds_for_bugs_in_esp32_en.pdf>`_ in ESP32 ECO3.

.. only:: not esp32

  .. list-table::
      :header-rows: 1
      :widths: 20 30 50
      :align: center

      * - Priority Level
        - Symbol
        - Remark
      * - 1
        - N/A
        - Exception and low priority interrupts, handled by ESP-IDF.
      * - 2-3
        - N/A
        - Medium priority interrupts, handled by ESP-IDF.
      * - 4
        - xt_highint4
        - Normally used by ESP-IDF debug logic.
      * - 5
        - xt_highint5
        - High priority interrupts, free to use.
      * - NMI
        - xt_nmi
        - Non-maskable interrupt, free to use.
      * - dbg
        - xt_debugexception
        - Debug exception. Called on e.g., a BREAK instruction.

Using these symbols is done by creating an assembly file with suffix ``.S`` and defining the named symbols, like this:

.. code-block:: none

        .section .iram1,"ax"
        .global     xt_highint4
        .type       xt_highint4,@function
        .align      4
    xt_highint5:
        ... your code here
        rsr     a0, EXCSAVE_5
        rfi     5

For a real-life example, see the :component_file:`esp_system/port/soc/{IDF_TARGET_PATH_NAME}/highint_hdl.S` file; the panic handler interrupt is implemented there.


Notes
-----

- Do not call C code from a high-priority interrupt; as these interrupts are run from a critical section, this can cause the target to crash. Note that although the panic handler interrupt does call normal C code, this exception is allowed due to the fact that this handler never returns (i.e., the application does not continue to run after the panic handler), so breaking C code execution flow is not a problem.

.. only:: esp32

  When :ref:`CONFIG_BTDM_CTRL_HLI` is enabled, C code is also called from a high-priority interrupt, this is possible thanks to some additional protection added to it.

- Make sure your assembly code gets linked in. Indeed, as the free-to-use symbols are declared as weak, the linker may discard the file containing the symbol. This happens if the only symbol defined, or used from the user file is the ``xt_*`` free-to-use symbol. To avoid this, in the assembly file containing the ``xt_*`` symbol, define another symbol, like:

.. code-block:: none

  .global ld_include_my_isr_file
  ld_include_my_isr_file:

Here it is called ``ld_include_my_isr_file`` but can have any name, as long as it is not defined anywhere else in the project.

Then, in the component ``CMakeLists.txt``, add this name as an unresolved symbol to the ld command line arguments:

.. code-block:: none

  target_link_libraries(${COMPONENT_TARGET} "-u ld_include_my_isr_file")

This will ensure the linker to always includes the file defining ``ld_include_my_isr_file``, so that the ISR is always linked.

- High-priority interrupts can be routed and handled using :cpp:func:`esp_intr_alloc` and associated functions. However, the handler and handler arguments to :cpp:func:`esp_intr_alloc` must be NULL.

- In theory, medium priority interrupts could also be handled in this way. ESP-IDF does not support this yet.

- To check Xtensa instruction set architecture (ISA), please refer to `Xtensa ISA Summary <https://www.cadence.com/content/dam/cadence-www/global/en_US/documents/tools/ip/tensilica-ip/isa-summary.pdf>`_.

See :example:`system/nmi_isr` for an example of how to implement a custom NMI handler on Xtensa-based targets.
````

## File: docs/en/api-guides/host-apps.rst
````
Running ESP-IDF Applications on Host
====================================

:link_to_translation:`zh_CN:[中文]`

.. note::

    Running ESP-IDF applications on host is currently still an experimental feature, thus there is no guarantee for API stability. However, user feedback via the `ESP-IDF GitHub repository <https://github.com/espressif/esp-idf>`_ or the `ESP32 forum <https://esp32.com/>`_ is highly welcome, and may help influence the future of design of the ESP-IDF host-based applications.

This document provides an overview of the methods to run ESP-IDF applications on Linux, and what type of ESP-IDF applications can typically be run on Linux.

Introduction
------------

Typically, an ESP-IDF application is built (cross-compiled) on a host machine, uploaded (i.e., flashed) to an ESP chip for execution, and monitored by the host machine via a UART/USB port. However, execution of an ESP-IDF application on an ESP chip can be limiting in various development/usage/testing scenarios.

Therefore, it is possible for an ESP-IDF application to be built and executed entirely within the same Linux host machine (henceforth referred to as "running on host"). Running ESP-IDF applications on host has several advantages:

- No need to upload to a target.
- Faster execution on a host machine, compared to running on an ESP chip.
- No requirements for any specific hardware, except the host machine itself.
- Easier automation and setup for software testing.
- Large number of tools for code and runtime analysis, e.g., Valgrind.

A large number of ESP-IDF components depend on chip-specific hardware. These hardware dependencies must be mocked or simulated when running on host. ESP-IDF currently supports the following mocking and simulation approaches:

1. Using the `FreeRTOS POSIX/Linux simulator <https://www.freertos.org/FreeRTOS-simulator-for-Linux.html>`_ that simulates FreeRTOS scheduling. On top of this simulation, other APIs are also simulated or implemented when running on host.
2. Using `CMock <https://www.throwtheswitch.org/cmock>`_ to mock all dependencies and run the code in complete isolation.

Note that despite the name, the FreeRTOS POSIX/Linux simulator currently also works on macOS. Running ESP-IDF applications on host machines is often used for testing. However, simulating the environment and mocking dependencies does not fully represent the target device. Thus, testing on the target device is still necessary, though with a different focus that usually puts more weight on integration and system testing.

.. note::

    Another possibility to run applications on the host is to use the QEMU simulator. However, QEMU development for ESP-IDF applications is still a work in progress and has not been documented yet.

CMock-Based Approach
^^^^^^^^^^^^^^^^^^^^

This approach uses the `CMock <https://www.throwtheswitch.org/cmock>`_ framework to solve the problem of missing hardware and software dependencies. CMock-based applications running on the host machine have the added advantage that they usually only compile the necessary code, i.e., the (mostly mocked) dependencies instead of the entire system. For a general introduction to Mocks and how to configure and use them in ESP-IDF, please refer to :ref:`mocks`.


POSIX/Linux Simulator Approach
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The `FreeRTOS POSIX/Linux simulator <https://www.freertos.org/FreeRTOS-simulator-for-Linux.html>`_ is available on ESP-IDF as a preview target already. This simulator allows ESP-IDF components to be implemented on the host, making them accessible to ESP-IDF applications when running on host. Currently, only a limited number of components are ready to be built on Linux. Furthermore, the functionality of each component ported to Linux may also be limited or different compared to the functionality when building that component for a chip target. For more information about whether the desired components are supported on Linux, please refer to :ref:`component-linux-mock-support`.

Note that this simulator relies heavily on POSIX signals and signal handlers to control and interrupt threads. Hence, it has the following *limitations*:

.. list::
    - Functions that are not *async-signal-safe*, e.g. ``printf()``, should be avoided. In particular, calling them from different tasks with different priority can lead to crashes and deadlocks.
    - Calling any FreeRTOS primitives from threads not created by FreeRTOS API functions is forbidden.
    - FreeRTOS tasks using any native blocking/waiting mechanism (e.g., ``select()``), may be perceived as *ready* by the simulated FreeRTOS scheduler and therefore may be scheduled, even though they are actually blocked. This is because the simulated FreeRTOS scheduler only recognizes tasks blocked on any FreeRTOS API as *waiting*.
    - APIs that may be interrupted by signals will continually receive the signals simulating FreeRTOS tick interrupts when invoked from a running simulated FreeRTOS task. Consequently, code that calls these APIs should be designed to handle potential interrupting signals or the API needs to be wrapped by the linker.

Since these limitations are not very practical, in particular for testing and development, we are currently evaluating if we can find a better solution for running ESP-IDF applications on the host machine.

Note furthermore that if you use the ESP-IDF FreeRTOS mock component (``tools/mocks/freertos``), these limitations do not apply. But that mock component will not do any scheduling, either.

.. only:: not esp32p4 and not esp32h4

    .. note::

        The FreeRTOS POSIX/Linux simulator allows configuring the :ref:`amazon_smp_freertos` version. However, the simulation still runs in single-core mode. The main reason allowing Amazon SMP FreeRTOS is to provide API compatibility with ESP-IDF applications written for Amazon SMP FreeRTOS.

Requirements for Using Mocks
----------------------------

.. include:: inc/linux-host-requirements.rst

If any mocks are used, then ``Ruby`` is required, too.

Build and Run
-------------

To build the application on Linux, the target has to be set to ``linux`` and then it can be built and run:

.. code-block:: bash

  idf.py --preview set-target linux
  idf.py build
  idf.py monitor

Troubleshooting
---------------

Since the applications are compiled for the host, they can be debugged with all the tools available on the host. E.g., this could be `GDB <https://man7.org/linux/man-pages/man1/gdb.1.html>`_ and `Valgrind <https://linux.die.net/man/1/valgrind>`_ on Linux. For cases where no debugger is attached, the segmentation fault and Abort signal handlers are customized to print additional information to the user and to increase compatibility with the ESP-IDF tools.

.. note::

  The following features are by no means a replacement for running the application in a debugger. It is only meant to give some additional information, e.g., if a battery of tests runs on Linux in a CI/CD system where only the application logs are collected. To trace down the actual issue in most cases, you will need to reproduce it with a debugger attached. A debugger is much more convenient too, because, for example, you do not need to convert addresses to line numbers.

Segmentation Faults
^^^^^^^^^^^^^^^^^^^

On Linux, applications prints an error message and a rudimentary backtrace once it encounters a segmentation fault. This information can be used to find the line numbers in the source code where the issue occurred. The following is an example of a segmentation fault in the Hello-World application:

.. code-block::

  ...
  Hello world!
  ERROR: Segmentation Fault, here's your backtrace:
  path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf(+0x2d1b)[0x55d3f636ad1b]
  /lib/x86_64-linux-gnu/libc.so.6(+0x3c050)[0x7f49f0e00050]
  path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf(+0x6198)[0x55d3f636e198]
  path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf(+0x5909)[0x55d3f636d909]
  path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf(+0x2c93)[0x55d3f636ac93]
  path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf(+0x484e)[0x55d3f636c84e]
  /lib/x86_64-linux-gnu/libc.so.6(+0x89134)[0x7f49f0e4d134]
  /lib/x86_64-linux-gnu/libc.so.6(+0x1097dc)[0x7f49f0ecd7dc]

Note that the addresses (``+0x...``) are relative binary addresses, which still need to be converted to the source code line numbers (see below).

Note furthermore that the backtrace is created from the signal handler, which means that the two uppermost stack frames are not of interest. Instead, the third line is the uppermost stack frame where the issue occurred:

.. code-block::

  path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf(+0x6198)[0x55d3f636e198]

To retrieve the actual line in the source code, we need to call the tool ``addr2line`` with the file name and the relative address (in this case ``+0x6198``):

.. code-block::

  $ addr2line -e path/to/esp-idf/examples/get-started/hello_world/build/hello_world.elf +0x6198
  path/to/esp-idf/components/esp_hw_support/port/linux/chip_info.c:13

From here on, you should use elaborate debugging tools available on the host to further trace the issue down.
For more information on ``addr2line`` and how to call it, see the `addr2line man page <https://linux.die.net/man/1/addr2line>`_.

Aborts
^^^^^^

Once ``abort()`` has been called, the following line is printed:

.. code-block::

  ERROR: Aborted

.. _component-linux-mock-support:

Component Linux/Mock Support Overview
-------------------------------------

Note that any "Yes" here does not necessarily mean a full implementation or mocking. It can also mean a partial implementation or mocking of functionality. Usually, the implementation or mocking is done to a point where enough functionality is provided to build and run a test application.

.. list-table::
   :header-rows: 1
   :widths: 20 10 10

   * - Component
     - Mock
     - Simulation
   * - cmock
     - No
     - Yes
   * - driver
     - Yes
     - No
   * - esp_app_format
     - No
     - Yes
   * - esp_common
     - No
     - Yes
   * - esp_event
     - Yes
     - Yes
   * - esp_http_client
     - No
     - Yes
   * - esp_http_server
     - No
     - Yes
   * - esp_https_server
     - No
     - Yes
   * - esp_hw_support
     - Yes
     - Yes
   * - esp_netif
     - Yes
     - Yes
   * - esp_netif_stack
     - No
     - Yes
   * - esp_partition
     - Yes
     - Yes
   * - esp_rom
     - No
     - Yes
   * - esp_system
     - No
     - Yes
   * - esp_timer
     - Yes
     - No
   * - esp_tls
     - Yes
     - Yes
   * - fatfs
     - No
     - Yes
   * - freertos
     - Yes
     - Yes
   * - hal
     - No
     - Yes
   * - heap
     - No
     - Yes
   * - http_parser
     - Yes
     - Yes
   * - json
     - No
     - Yes
   * - linux
     - No
     - Yes
   * - log
     - No
     - Yes
   * - lwip
     - Yes
     - Yes
   * - mbedtls
     - No
     - Yes
   * - mqtt
     - No
     - Yes
   * - nvs_flash
     - No
     - Yes
   * - partition_table
     - No
     - Yes
   * - protobuf-c
     - No
     - Yes
   * - pthread
     - No
     - Yes
   * - soc
     - No
     - Yes
   * - spiffs
     - No
     - Yes
   * - spi_flash
     - Yes
     - No
   * - tcp_transport
     - Yes
     - No
   * - unity
     - No
     - Yes
````

## File: docs/en/api-guides/index.rst
````
API Guides
**********
:link_to_translation:`zh_CN:[中文]`

.. toctree::
   :maxdepth: 1

   app_trace
   startup
   :SOC_BT_SUPPORTED: bt-architecture/index
   :SOC_BT_CLASSIC_SUPPORTED: classic-bt/index
   :SOC_BLE_SUPPORTED: ble/index
   :SOC_BLE_MESH_SUPPORTED: esp-ble-mesh/ble-mesh-index
   bootloader
   build-system
   :SOC_SUPPORT_COEXISTENCE: coexist
   c
   cplusplus
   code-quality/index
   core_dump
   current-consumption-measurement-modules
   :ESP_ROM_SUPPORT_DEEP_SLEEP_WAKEUP_STUB: deep-sleep-stub
   :SOC_USB_OTG_SUPPORTED: dfu
   error-handling
   :SOC_WIFI_MESH_SUPPORT: esp-wifi-mesh
   :SOC_SPIRAM_SUPPORTED: external-ram
   fatal-errors
   file-system-considerations
   :esp32s3: flash_psram_config
   hardware-abstraction
   :CONFIG_IDF_TARGET_ARCH_XTENSA: hlinterrupts
   jtag-debugging/index
   kconfig/index
   linker-script-generation
   low-power-mode/index
   lwip
   memory-types
   openthread
   partition-tables
   performance/index
   reproducible-builds
   :(SOC_WIFI_SUPPORTED or SOC_BT_SUPPORTED or SOC_IEEE802154_SUPPORTED): RF_calibration
   stdio
   thread-local-storage
   tools/index
   unit-tests
   host-apps
   :SOC_USB_OTG_SUPPORTED and not esp32p4: usb-otg-console
   :SOC_USB_SERIAL_JTAG_SUPPORTED: usb-serial-jtag-console
   :SOC_WIFI_SUPPORTED: wifi
   :SOC_WIFI_SUPPORTED: wifi-security
   wifi-expansion
   :SOC_WIFI_SUPPORTED or SOC_BT_SUPPORTED or SOC_IEEE802154_SUPPORTED: phy
````

## File: docs/en/api-guides/linker-script-generation.rst
````
Linker Script Generation
========================

:link_to_translation:`zh_CN:[中文]`

Overview
--------

There are several :ref:`memory regions <memory-layout>` where code and data can be placed. Code and read-only data are placed by default in flash, writable data in RAM, etc. However, it is sometimes necessary to change these default placements.

For example, it may be necessary to place:

.. list::

    * critical code in RAM for performance reasons.
    * executable code in IRAM so that it can be ran while cache is disabled.
    :ESP_ROM_SUPPORT_DEEP_SLEEP_WAKEUP_STUB: * code in RTC memory for use in a wake stub.
    :SOC_ULP_SUPPORTED: * code in RTC memory for use by the ULP coprocessor.

With the linker script generation mechanism, it is possible to specify these placements at the component level within ESP-IDF. The component presents information on how it would like to place its symbols, objects or the entire archive. During build, the information presented by the components are collected, parsed and processed; and the placement rules generated is used to link the app.

Quick Start
------------

This section presents a guide for quickly placing code/data to RAM and RTC memory - placements ESP-IDF provides out-of-the-box.

For this guide, suppose we have the following::

    components
    └── my_component
        ├── CMakeLists.txt
        ├── Kconfig
        ├── src/
        │   ├── my_src1.c
        │   ├── my_src2.c
        │   └── my_src3.c
        └── my_linker_fragment_file.lf

- a component named ``my_component`` that is archived as library ``libmy_component.a`` during build
- three source files archived under the library, ``my_src1.c``, ``my_src2.c`` and ``my_src3.c`` which are compiled as ``my_src1.o``, ``my_src2.o`` and ``my_src3.o``, respectively
- under ``my_src1.o``, the function ``my_function1`` is defined; under ``my_src2.o``, the function ``my_function2`` is defined
- there is bool-type config ``PERFORMANCE_MODE`` (y/n) and int type config ``PERFORMANCE_LEVEL`` (with range 0-3) in ``my_component``'s Kconfig

Creating and Specifying a Linker Fragment File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Before anything else, a linker fragment file needs to be created. A linker fragment file is simply a text file with a ``.lf`` extension upon which the desired placements will be written. After creating the file, it is then necessary to present it to the build system. The instructions for the build systems supported by ESP-IDF are as follows:

In the component's ``CMakeLists.txt`` file, specify argument ``LDFRAGMENTS`` in the ``idf_component_register`` call. The value of ``LDFRAGMENTS`` can either be an absolute path or a relative path from the component directory to the created linker fragment file.

.. code-block:: cmake

    # file paths relative to CMakeLists.txt
    idf_component_register(...
                           LDFRAGMENTS "path/to/linker_fragment_file.lf" "path/to/another_linker_fragment_file.lf"
                           ...
                           )


Specifying Placements
^^^^^^^^^^^^^^^^^^^^^

It is possible to specify placements at the following levels of granularity:

    - object file (``.obj`` or ``.o`` files)
    - symbol (function/variable)
    - archive (``.a`` files)

.. _ldgen-placing-object-files :

Placing Object Files
""""""""""""""""""""

Suppose the entirety of ``my_src1.o`` is performance-critical, so it is desirable to place it in RAM. On the other hand, the entirety of ``my_src2.o`` contains symbols needed coming out of deep sleep, so it needs to be put under RTC memory.

In the linker fragment file, we can write:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        my_src1 (noflash)     # places all my_src1 code/read-only data under IRAM/DRAM
        my_src2 (rtc)         # places all my_src2 code/ data and read-only data under RTC fast memory/RTC slow memory

What happens to ``my_src3.o``? Since it is not specified, default placements are used for ``my_src3.o``. More on default placements :ref:`here <ldgen-default-placements>`.

Placing Symbols
""""""""""""""""

Continuing our example, suppose that among functions defined under ``object1.o``, only ``my_function1`` is performance-critical; and under ``object2.o``, only ``my_function2`` needs to execute after the chip comes out of deep sleep. This could be accomplished by writing:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        my_src1:my_function1 (noflash)
        my_src2:my_function2 (rtc)

The default placements are used for the rest of the functions in ``my_src1.o`` and ``my_src2.o`` and the entire ``object3.o``. Something similar can be achieved for placing data by writing the variable name instead of the function name, like so::

       my_src1:my_variable (noflash)

.. warning::

    There are :ref:`limitations<ldgen-symbol-granularity-placements>` in placing code/data at symbol granularity. In order to ensure proper placements, an alternative would be to group relevant code and data into source files, and :ref:`use object-granularity placements<ldgen-placing-object-files>`.

Placing Entire Archive
"""""""""""""""""""""""

In this example, suppose that the entire component archive needs to be placed in RAM. This can be written as:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        * (noflash)

Similarly, this places the entire component in RTC memory:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        * (rtc)


.. _ldgen-conditional-placements:

Configuration-Dependent Placements
""""""""""""""""""""""""""""""""""

Suppose that the entire component library should only have special placement when a certain condition is true; for example, when ``CONFIG_PERFORMANCE_MODE == y``. This could be written as:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        if PERFORMANCE_MODE = y:
            * (noflash)
        else:
            * (default)

For a more complex config-dependent placement, suppose the following requirements: when ``CONFIG_PERFORMANCE_LEVEL == 1``, only ``object1.o`` is put in RAM; when ``CONFIG_PERFORMANCE_LEVEL == 2``, ``object1.o`` and ``object2.o``; and when ``CONFIG_PERFORMANCE_LEVEL == 3`` all object files under the archive are to be put into RAM. When these three are false however, put entire library in RTC memory. This scenario is a bit contrived, but, it can be written as:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        if PERFORMANCE_LEVEL = 1:
            my_src1 (noflash)
        elif PERFORMANCE_LEVEL = 2:
            my_src1 (noflash)
            my_src2 (noflash)
        elif PERFORMANCE_LEVEL = 3:
            my_src1 (noflash)
            my_src2 (noflash)
            my_src3 (noflash)
        else:
            * (rtc)

Nesting condition-checking is also possible. The following is equivalent to the snippet above:

.. code-block:: none

    [mapping:my_component]
    archive: libmy_component.a
    entries:
        if PERFORMANCE_LEVEL <= 3 && PERFORMANCE_LEVEL > 0:
            if PERFORMANCE_LEVEL >= 1:
                object1 (noflash)
                if PERFORMANCE_LEVEL >= 2:
                    object2 (noflash)
                    if PERFORMANCE_LEVEL >= 3:
                        object2 (noflash)
        else:
            * (rtc)

.. _ldgen-default-placements:

The 'default' Placements
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Up until this point, the term  'default placements' has been mentioned as fallback placements when the placement rules ``rtc`` and ``noflash`` are not specified. It is important to note that the tokens ``noflash`` or ``rtc`` are not merely keywords, but are actually entities called fragments, specifically :ref:`schemes <ldgen-scheme-fragment>`.

In the same manner as ``rtc`` and ``noflash`` are schemes, there exists a ``default`` scheme which defines what the default placement rules should be. As the name suggests, it is where code and data are usually placed, i.e., code/constants is placed in flash, variables placed in RAM, etc.  More on the default scheme :ref:`here <ldgen-default-scheme>`.

.. note::

    For an example of an ESP-IDF component using the linker script generation mechanism, see :component_file:`freertos/CMakeLists.txt`. ``freertos`` uses this to place its object files to the instruction RAM for performance reasons.

This marks the end of the quick start guide. The following text discusses the internals of the mechanism in a little bit more detail. The following sections should be helpful in creating custom placements or modifying default behavior.

Linker Script Generation Internals
----------------------------------

Linking is the last step in the process of turning C/C++ source files into an executable. It is performed by the toolchain's linker, and accepts linker scripts which specify code/data placements, among other things. With the linker script generation mechanism, this process is no different, except that the linker script passed to the linker is dynamically generated from: (1) the collected :ref:`linker fragment files <ldgen-linker-fragment-files>` and (2) :ref:`linker script template <ldgen-linker-script-template>`.

.. note::

    The tool that implements the linker script generation mechanism lives under :idf:`tools/ldgen`.

.. _ldgen-linker-fragment-files :

Linker Fragment Files
^^^^^^^^^^^^^^^^^^^^^

As mentioned in the quick start guide, fragment files are simple text files with the ``.lf`` extension containing the desired placements. This is a simplified description of what fragment files contain, however. What fragment files actually contain are 'fragments'. Fragments are entities which contain pieces of information which, when put together, form placement rules that tell where to place sections of object files in the output binary. There are three types of fragments: :ref:`sections <ldgen-sections-fragment>`, :ref:`scheme <ldgen-scheme-fragment>` and :ref:`mapping <ldgen-mapping-fragment>`.

Grammar
"""""""

The three fragment types share a common grammar:

.. code-block:: none

    [type:name]
    key: value
    key:
        value
        value
        value
        ...

- type: Corresponds to the fragment type, can either be ``sections``, ``scheme`` or ``mapping``.
- name: The name of the fragment, should be unique for the specified fragment type.
- key, value: Contents of the fragment; each fragment type may support different keys and different grammars for the key values.

    - For :ref:`sections <ldgen-sections-fragment>` and :ref:`scheme <ldgen-scheme-fragment>`, the only supported key is ``entries``
    - For :ref:`mappings <ldgen-mapping-fragment>`, both ``archive`` and ``entries`` are supported.

.. note::

    In cases where multiple fragments of the same type and name are encountered, an exception is thrown.

.. note::

    The only valid characters for fragment names and keys are alphanumeric characters and underscore.

.. _ldgen-condition-checking :

**Condition Checking**

Condition checking enable the linker script generation to be configuration-aware. Depending on whether expressions involving configuration values are true or not, a particular set of values for a key can be used. The evaluation uses ``eval_string`` from kconfiglib package and adheres to its required syntax and limitations. Supported operators are as follows:

    - comparison
        - LessThan ``<``
        - LessThanOrEqualTo ``<=``
        - MoreThan ``>``
        - MoreThanOrEqualTo ``>=``
        - Equal ``=``
        - NotEqual ``!=``
    - logical
        - Or ``||``
        - And ``&&``
        - Negation ``!``
    - grouping
        - Parenthesis ``()``

Condition checking behaves as you would expect an ``if...elseif/elif...else`` block in other languages. Condition-checking is possible for both key values and entire fragments. The two sample fragments below are equivalent:

.. code-block:: none

    # Value for keys is dependent on config
    [type:name]
    key_1:
        if CONDITION = y:
            value_1
        else:
            value_2
    key_2:
        if CONDITION = y:
            value_a
        else:
            value_b

.. code-block:: none

    # Entire fragment definition is dependent on config
    if CONDITION = y:
        [type:name]
        key_1:
            value_1
        key_2:
            value_a
    else:
        [type:name]
        key_1:
            value_2
        key_2:
            value_b

**Comments**

Comment in linker fragment files begin with ``#``. Like in other languages, comment are used to provide helpful descriptions and documentation and are ignored during processing.

Types
"""""

.. _ldgen-sections-fragment :

**Sections**

Sections fragments defines a list of object file sections that the GCC compiler emits. It may be a default section (e.g., ``.text``, ``.data``) or it may be user defined section through the ``__attribute__`` keyword.

The use of an optional '+' indicates the inclusion of the section in the list, as well as sections that start with it. This is the preferred method over listing both explicitly.

.. code-block:: none

    [sections:name]
    entries:
        .section+
        .section
        ...

Example:

.. code-block:: none

    # Non-preferred
    [sections:text]
    entries:
        .text
        .text.*
        .literal
        .literal.*

    # Preferred, equivalent to the one above
    [sections:text]
    entries:
        .text+              # means .text and .text.*
        .literal+           # means .literal and .literal.*

.. _ldgen-scheme-fragment :

**Scheme**

Scheme fragments define what ``target`` a sections fragment is assigned to.

.. code-block:: none

    [scheme:name]
    entries:
        sections -> target
        sections -> target
        ...

Example:

.. code-block:: none

    [scheme:noflash]
    entries:
        text -> iram0_text          # the entries under the sections fragment named text will go to iram0_text
        rodata -> dram0_data        # the entries under the sections fragment named rodata will go to dram0_data

.. _ldgen-default-scheme:

The ``default`` scheme

There exists a special scheme with the name ``default``. This scheme is special because catch-all placement rules are generated from its entries. This means that, if one of its entries is ``text -> flash_text``, the placement rule will be generated for the target ``flash_text``.

.. code-block:: none

    *(.literal .literal.* .text .text.*)

These catch-all rules then effectively serve as fallback rules for those whose mappings were not specified.


The ``default scheme`` is defined in :component_file:`esp_system/app.lf`. The ``noflash`` and ``rtc`` scheme fragments which are
built-in schemes referenced in the quick start guide are also defined in this file.


.. _ldgen-mapping-fragment :

**Mapping**

Mapping fragments define what scheme fragment to use for mappable entities, i.e., object files, function names, variable names, archives.

.. code-block:: none

    [mapping:name]
    archive: archive                # output archive file name, as built (i.e., libxxx.a)
    entries:
        object:symbol (scheme)      # symbol granularity
        object (scheme)             # object granularity
        * (scheme)                  # archive granularity

There are three levels of placement granularity:

    - symbol: The object file name and symbol name are specified. The symbol name can be a function name or a variable name.
    - object: Only the object file name is specified.
    - archive: ``*`` is specified, which is a short-hand for all the object files under the archive.

To know what an entry means, let us expand a sample object-granularity placement:

.. code-block:: none

    object (scheme)

Then expanding the scheme fragment from its entries definitions, we have:

.. code-block:: none

    object (sections -> target,
            sections -> target,
            ...)

Expanding the sections fragment with its entries definition:

.. code-block:: none

    object (.section,      # given this object file
            .section,      # put its sections listed here at this
            ... -> target, # target

            .section,
            .section,      # same should be done for these sections
            ... -> target,

            ...)           # and so on

Example:

.. code-block:: none

    [mapping:map]
    archive: libfreertos.a
    entries:
        * (noflash)

Aside from the entity and scheme, flags can also be specified in an entry. The following flags are supported (note: <> = argument name, [] = optional):

1. ALIGN(<alignment>[, pre, post])

    Align the placement by the amount specified in ``alignment``. Generates

.. code-block::none

    . = ALIGN(<alignment>)

    before and/or after (depending whether ``pre``, ``post`` or both are specified) the input section description generated from the mapping fragment entry. If neither 'pre' or 'post' is specified, the alignment command is generated before the input section description. Order sensitive.

2. SORT([<sort_by_first>, <sort_by_second>])

    Emits ``SORT_BY_NAME``, ``SORT_BY_ALIGNMENT``, ``SORT_BY_INIT_PRIORITY`` or ``SORT`` in the input section description.

    Possible values for ``sort_by_first`` and ``sort_by_second`` are: ``name``, ``alignment``, ``init_priority``.

    If both ``sort_by_first`` and ``sort_by_second`` are not specified, the input sections are sorted by name. If both are specified, then the nested sorting follows the same rules discussed in https://sourceware.org/binutils/docs/ld/Input-Section-Wildcards.html.

3. KEEP()

    Prevent the linker from discarding the placement by surrounding the input section description with KEEP command. See https://sourceware.org/binutils/docs/ld/Input-Section-Keep.html for more details.

4.SURROUND(<name>)

    Generate symbols before and after the placement. The generated symbols follow the naming ``_<name>_start`` and ``_<name>_end``. For example, if ``name`` == sym1,

.. code-block::none

    _sym1_start = ABSOLUTE(.)
    ...
    _sym2_end = ABSOLUTE(.)

    These symbols can then be referenced from C/C++ code. Order sensitive.

When adding flags, the specific ``section -> target`` in the scheme needs to be specified. For multiple ``section -> target``, use a comma as a separator. For example,

.. code-block:: none

    # Notes:
    # A. semicolon after entity-scheme
    # B. comma before section2 -> target2
    # C. section1 -> target1 and section2 -> target2 should be defined in entries of scheme1
    entity1 (scheme1);
        section1 -> target1 KEEP() ALIGN(4, pre, post),
        section2 -> target2 SURROUND(sym) ALIGN(4, post) SORT()

Putting it all together, the following mapping fragment, for example,

.. code-block:: none

    [mapping:name]
    archive: lib1.a
    entries:
        obj1 (noflash);
            rodata -> dram0_data KEEP() SORT() ALIGN(8) SURROUND(my_sym)

generates an output on the linker script:

.. code-block:: none

    . = ALIGN(8)
    _my_sym_start = ABSOLUTE(.)
    KEEP(lib1.a:obj1.*( SORT(.rodata) SORT(.rodata.*) ))
    _my_sym_end = ABSOLUTE(.)

Note that ALIGN and SURROUND, as mentioned in the flag descriptions, are order sensitive. Therefore, if for the same mapping fragment these two are switched, the following is generated instead:

.. code-block:: none

    _my_sym_start = ABSOLUTE(.)
    . = ALIGN(8)
    KEEP(lib1.a:obj1.*( SORT(.rodata) SORT(.rodata.*) ))
    _my_sym_end = ABSOLUTE(.)

.. _ldgen-symbol-granularity-placements :

On Symbol-Granularity Placements
""""""""""""""""""""""""""""""""

Symbol granularity placements is possible due to compiler flags ``-ffunction-sections`` and ``-ffdata-sections``. ESP-IDF compiles with these flags by default.
If the user opts to remove these flags, then the symbol-granularity placements will not work. Furthermore, even with the presence of these flags, there are still other limitations to keep in mind due to the dependence on the compiler's emitted output sections.

For example, with ``-ffunction-sections``, separate sections are emitted for each function; with section names predictably constructed i.e., ``.text.{func_name}`` and ``.literal.{func_name}``. This is not the case for string literals within the function, as they go to pooled or generated section names.

With ``-fdata-sections``, for global scope data the compiler predictably emits either ``.data.{var_name}``, ``.rodata.{var_name}`` or ``.bss.{var_name}``; and so ``Type I`` mapping entry works for these.
However, this is not the case for static data declared in function scope, as the generated section name is a result of mangling the variable name with some other information.

.. _ldgen-linker-script-template :

Linker Script Template
^^^^^^^^^^^^^^^^^^^^^^

The linker script template is the skeleton in which the generated placement rules are put into. It is an otherwise ordinary linker script, with a specific marker syntax that indicates where the generated placement rules are placed.

To reference the placement rules collected under a ``target`` token, the following syntax is used:

.. only:: SOC_MEM_NON_CONTIGUOUS_SRAM

    .. code-block:: none

        arrays[target]      /* refers to objects under the SURROUND keyword */
        mapping[target]     /* refers to all other data */

.. only:: not SOC_MEM_NON_CONTIGUOUS_SRAM

    .. code-block:: none

        mapping[target]

Example:

The example below is an excerpt from a possible linker script template. It defines an output section ``.iram0.text``, and inside is a marker referencing the target ``iram0_text``.

.. only:: SOC_MEM_NON_CONTIGUOUS_SRAM

    .. code-block:: none

        .iram0.text :
        {
            /* Code marked as running out of IRAM */
            _iram_text_start = ABSOLUTE(.);

            /* Markers referencing iram0_text */
            arrays[iram0_text]
            mapping[iram0_text]

            _iram_text_end = ABSOLUTE(.);
        } > iram0_0_seg

.. only:: not SOC_MEM_NON_CONTIGUOUS_SRAM

    .. code-block:: none

        .iram0.text :
        {
            /* Code marked as running out of IRAM */
            _iram_text_start = ABSOLUTE(.);

            /* Marker referencing iram0_text */
            mapping[iram0_text]

            _iram_text_end = ABSOLUTE(.);
        } > iram0_0_seg

Suppose the generator collected the fragment definitions below:

.. code-block:: none

    [sections:text]
        .text+
        .literal+

    [sections:iram]
        .iram1+

    [scheme:default]
    entries:
        text -> flash_text
        iram -> iram0_text

    [scheme:noflash]
    entries:
        text -> iram0_text

    [mapping:freertos]
    archive: libfreertos.a
    entries:
        * (noflash)

Then the corresponding excerpt from the generated linker script will be as follows:

.. code-block:: c

    .iram0.text :
    {
        /* Code marked as running out of IRAM */
        _iram_text_start = ABSOLUTE(.);

        /* Placement rules generated from the processed fragments, placed where the marker was in the template */
        *(.iram1 .iram1.*)
        *libfreertos.a:(.literal .text .literal.* .text.*)

        _iram_text_end = ABSOLUTE(.);
    } > iram0_0_seg

``*libfreertos.a:(.literal .text .literal.* .text.*)``

    Rule generated from the entry ``* (noflash)`` of the ``freertos`` mapping fragment. All ``text`` sections of all object files under the archive ``libfreertos.a`` will be collected under the target ``iram0_text`` (as per the ``noflash`` scheme) and placed wherever in the template ``iram0_text`` is referenced by a marker.

``*(.iram1 .iram1.*)``

    Rule generated from the default scheme entry ``iram -> iram0_text``. Since the default scheme specifies an ``iram -> iram0_text`` entry, it too is placed wherever ``iram0_text`` is referenced by a marker. Since it is a rule generated from the default scheme, it comes first among all other rules collected under the same target name.

    The linker script template currently used is :component_file:`esp_system/ld/{IDF_TARGET_PATH_NAME}/sections.ld.in`; the generated output script ``sections.ld`` is put under its build directory.
````

## File: docs/en/api-guides/lwip.rst
````
lwIP
====

:link_to_translation:`zh_CN:[中文]`

ESP-IDF uses the open source `lwIP lightweight TCP/IP stack`_. The ESP-IDF version of lwIP (`esp-lwip`_) has some modifications and additions compared to the upstream project.

Supported APIs
--------------

ESP-IDF supports the following lwIP TCP/IP stack functions:

- `BSD Sockets API`_
- `Netconn API`_ is enabled but not officially supported for ESP-IDF applications

.. _lwip-dns-limitation:

Adapted APIs
^^^^^^^^^^^^

    .. warning::

        When using any lwIP API other than the `BSD Sockets API`_, please make sure that the API is thread-safe. To check if a given API call is thread-safe, enable the :ref:`CONFIG_LWIP_CHECK_THREAD_SAFETY` configuration option and run the application. This enables lwIP to assert the correct access of the TCP/IP core functionality. If the API is not accessed or locked properly from the appropriate `lwIP FreeRTOS Task`_, the execution will be aborted. The general recommendation is to use the :doc:`/api-reference/network/esp_netif` component to interact with lwIP.

Some common lwIP app APIs are supported indirectly by ESP-IDF:

- Dynamic Host Configuration Protocol (DHCP) Server & Client are supported indirectly via the :doc:`/api-reference/network/esp_netif` functionality.
- Domain Name System (DNS) is supported in lwIP; DNS servers could be assigned automatically when acquiring a DHCP address, or manually configured using the :doc:`/api-reference/network/esp_netif` API.

.. note::

    DNS server configuration in lwIP is global, not interface-specific. If you are using multiple network interfaces with distinct DNS servers, exercise caution to prevent inadvertent overwrites of one interface's DNS settings when acquiring a DHCP lease from another interface.

- Simple Network Time Protocol (SNTP) is also supported via the :doc:`/api-reference/network/esp_netif`, or directly via the :component_file:`lwip/include/apps/esp_sntp.h` functions, which also provide thread-safe API to :component_file:`lwip/lwip/src/include/lwip/apps/sntp.h` functions, see also :ref:`system-time-sntp-sync`. For implementation details, see :example:`protocols/sntp`. This example demonstrates how to use the LwIP SNTP module to obtain time from internet servers, configure the synchronization method and interval, and retrieve time using the SNTP-over-DHCP module.
- ICMP Ping is supported using a variation on the lwIP ping API, see :doc:`/api-reference/protocols/icmp_echo`.
- ICMPv6 Ping, supported by lwIP's ICMPv6 Echo API, is used to test IPv6 network connectivity. For more information, see :example:`protocols/sockets/icmpv6_ping`. This example demonstrates how to use the network interface to discover an IPv6 address, create a raw ICMPv6 socket, send an ICMPv6 Echo Request to a destination IPv6 address, and wait for an Echo Reply from the target.
- NetBIOS lookup is available using the standard lwIP API. :example:`protocols/http_server/restful_server` has the option to demonstrate using NetBIOS to look up a host on the LAN.
- mDNS uses a different implementation to the lwIP default mDNS, see :doc:`/api-reference/protocols/mdns`. But lwIP can look up mDNS hosts using standard APIs such as ``gethostbyname()`` and the convention ``hostname.local``, provided the :ref:`CONFIG_LWIP_DNS_SUPPORT_MDNS_QUERIES` setting is enabled.
- The PPP implementation in lwIP can be used to create PPPoS (PPP over serial) interface in ESP-IDF. Please refer to the documentation of the :doc:`/api-reference/network/esp_netif` component to create and configure a PPP network interface, by means of the ``ESP_NETIF_DEFAULT_PPP()`` macro defined in :component_file:`esp_netif/include/esp_netif_defaults.h`. Additional runtime settings are provided via :component_file:`esp_netif/include/esp_netif_ppp.h`. PPPoS interfaces are typically used to interact with NBIoT/GSM/LTE modems. More application-level friendly API is supported by the `esp_modem <https://components.espressif.com/component/espressif/esp_modem>`_ library, which uses this PPP lwIP module behind the scenes.

BSD Sockets API
---------------

The BSD Sockets API is a common cross-platform TCP/IP sockets API that originated in the Berkeley Standard Distribution of UNIX but is now standardized in a section of the POSIX specification. BSD Sockets are sometimes called POSIX Sockets or Berkeley Sockets.

As implemented in ESP-IDF, lwIP supports all of the common usages of the BSD Sockets API. However, not all operations are fully thread-safe, and simultaneous reads and writes from multiple threads may require additional synchronization mechanisms, see :ref:`lwip-limitations` for more details.

References
^^^^^^^^^^

A wide range of BSD Sockets reference materials are available, including:

- `Single UNIX Specification - BSD Sockets page <https://pubs.opengroup.org/onlinepubs/007908799/xnsix.html>`_
- `Berkeley Sockets - Wikipedia page <https://en.wikipedia.org/wiki/Berkeley_sockets>`_

Application Examples
^^^^^^^^^^^^^^^^^^^^

A number of ESP-IDF examples show how to use the BSD Sockets APIs:

- :example:`protocols/sockets/non_blocking` demonstrates how to configure and run a non-blocking TCP client and server, supporting both IPv4 and IPv6 protocols.

- :example:`protocols/sockets/tcp_server` demonstrates how to create a TCP server that accepts client connection requests and receives data.

- :example:`protocols/sockets/tcp_client` demonstrates how to create a TCP client that connects to a server using a predefined IP address and port.

- :example:`protocols/sockets/tcp_client_multi_net` demonstrates how to use Ethernet and Wi-Fi interfaces together, connect to both simultaneously, create a TCP client for each interface, and send a basic HTTP request and response.

- :example:`protocols/sockets/udp_server` demonstrates how to create a UDP server that receives client connection requests and data.

- :example:`protocols/sockets/udp_client` demonstrates how to create a UDP client that connects to a server using a predefined IP address and port.

- :example:`protocols/sockets/udp_multicast` demonstrates how to use the IPV4 and IPV6 UDP multicast features via the BSD-style sockets interface.

Supported Functions
^^^^^^^^^^^^^^^^^^^

The following BSD socket API functions are supported. For full details, see :component_file:`lwip/lwip/src/include/lwip/sockets.h`.

- ``socket()``
- ``bind()``
- ``accept()``
- ``shutdown()``
- ``getpeername()``
- ``getsockopt()`` & ``setsockopt()``: see `Socket Options`_
- ``close()``: via :doc:`/api-reference/storage/vfs`
- ``read()``, ``readv()``, ``write()``, ``writev()``: via :doc:`/api-reference/storage/vfs`
- ``recv()``, ``recvmsg()``, ``recvfrom()``
- ``send()``, ``sendmsg()``, ``sendto()``
- ``select()``: via :doc:`/api-reference/storage/vfs`
- ``poll()`` : on ESP-IDF, ``poll()`` is implemented by calling ``select()`` internally, so using ``select()`` directly is recommended, if a choice of methods is available
- ``fcntl()``: see `fcntl()`_

Non-standard functions:

- ``ioctl()``: see `ioctl()`_

.. note::

  Some lwIP application sample code uses prefixed versions of BSD APIs, e.g., ``lwip_socket()``, instead of the standard ``socket()``. Both forms can be used with ESP-IDF, but using standard names is recommended.

Socket Error Handling
^^^^^^^^^^^^^^^^^^^^^

BSD Socket error handling code is very important for robust socket applications. Normally, socket error handling involves the following aspects:

- Detecting the error
- Getting the error reason code
- Handling the error according to the reason code

In lwIP, we have two different scenarios for handling socket errors:

- Socket API returns an error. For more information, see `Socket API Errors`_.
- ``select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)`` has an exception descriptor indicating that the socket has an error. For more information, see `select() Errors`_.

Socket API Errors
+++++++++++++++++

**Error detection**

  - We can know that the socket API fails according to its return value.

**Get the error reason code**

  - When socket API fails, the return value does not contain the failure reason and the application can get the error reason code by accessing ``errno``. Different values indicate different meanings. For more information, see `Socket Error Reason Code`_.

Example:

.. code-block::

        int err;
        int sockfd;

        if (sockfd = socket(AF_INET,SOCK_STREAM,0) < 0) {
            // the error code is obtained from errno
            err = errno;
            return err;
        }

``select()`` Errors
+++++++++++++++++++

**Error detection**

  - Socket error when ``select()`` has exception descriptor.

**Get the error reason code**

  - If the ``select()`` indicates that the socket fails, we can not get the error reason code by accessing ``errno``, instead we should call ``getsockopt()`` to get the failure reason code. Since ``select()`` has exception descriptor, the error code is not given to ``errno``.

.. note::

    The ``getsockopt()`` function has the following prototype: ``int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)``. Its purpose is to get the current value of the option of any type, any state socket, and store the result in ``optval``. For example, when you get the error code on a socket, you can get it by ``getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &optlen)``.

Example:

.. code-block::

        int err;

        if (select(sockfd + 1, NULL, NULL, &exfds, &tval) <= 0) {
            err = errno;
            return err;
        } else {
            if (FD_ISSET(sockfd, &exfds)) {
                // select() exception set using getsockopt()
                int optlen = sizeof(int);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &optlen);
                return err;
            }
        }

Socket Error Reason Code
++++++++++++++++++++++++

Below is a list of common error codes. For a more detailed list of standard POSIX/C error codes, please see `newlib errno.h <https://github.com/espressif/newlib-esp32/blob/master/newlib/libc/include/sys/errno.h>`_ and the platform-specific extensions :component_file:`esp_libc/platform_include/sys/errno.h`.

.. list-table::
    :header-rows: 1
    :widths: 50 50
    :align: center

    * - Error code
      - Description
    * - ECONNREFUSED
      - Connection refused
    * - EADDRINUSE
      - Address already in use
    * - ECONNABORTED
      - Software caused connection abort
    * - ENETUNREACH
      - Network is unreachable
    * - ENETDOWN
      - Network interface is not configured
    * - ETIMEDOUT
      - Connection timed out
    * - EHOSTDOWN
      - Host is down
    * - EHOSTUNREACH
      - Host is unreachable
    * - EINPROGRESS
      - Connection already in progress
    * - EALREADY
      - Socket already connected
    * - EDESTADDRREQ
      - Destination address required
    * - EPROTONOSUPPORT
      - Unknown protocol

Socket Options
^^^^^^^^^^^^^^

The ``getsockopt()`` and ``setsockopt()`` functions allow getting and setting per-socket options respectively.

Not all standard socket options are supported by lwIP in ESP-IDF. The following socket options are supported:

Common Options
++++++++++++++

Used with level argument ``SOL_SOCKET``.

- ``SO_REUSEADDR``: available if :ref:`CONFIG_LWIP_SO_REUSE` is set, whose behavior can be customized by setting :ref:`CONFIG_LWIP_SO_REUSE_RXTOALL`
- ``SO_KEEPALIVE``
- ``SO_BROADCAST``
- ``SO_ACCEPTCONN``
- ``SO_RCVBUF``: available if :ref:`CONFIG_LWIP_SO_RCVBUF` is set
- ``SO_SNDTIMEO`` / ``SO_RCVTIMEO``
- ``SO_ERROR``: only used with ``select()``, see `Socket Error Handling`_
- ``SO_TYPE``
- ``SO_NO_CHECK``: for UDP sockets only

IP Options
++++++++++

Used with level argument ``IPPROTO_IP``.

- ``IP_TOS``
- ``IP_TTL``
- ``IP_PKTINFO``: available if :ref:`CONFIG_LWIP_NETBUF_RECVINFO` is set

For multicast UDP sockets:

- ``IP_MULTICAST_IF``
- ``IP_MULTICAST_LOOP``
- ``IP_MULTICAST_TTL``
- ``IP_ADD_MEMBERSHIP``
- ``IP_DROP_MEMBERSHIP``

TCP Options
+++++++++++

TCP sockets only. Used with level argument ``IPPROTO_TCP``.

- ``TCP_NODELAY``

Options relating to TCP keepalive probes:

- ``TCP_KEEPALIVE``: int value, TCP keepalive period in milliseconds
- ``TCP_KEEPIDLE``: same as ``TCP_KEEPALIVE``, but the value is in seconds
- ``TCP_KEEPINTVL``: int value, the interval between keepalive probes in seconds
- ``TCP_KEEPCNT``: int value, number of keepalive probes before timing out

IPv6 Options
++++++++++++

IPv6 sockets only. Used with level argument ``IPPROTO_IPV6``.

- ``IPV6_CHECKSUM``
- ``IPV6_V6ONLY``

For multicast IPv6 UDP sockets:

- ``IPV6_JOIN_GROUP`` / ``IPV6_ADD_MEMBERSHIP``
- ``IPV6_LEAVE_GROUP`` / ``IPV6_DROP_MEMBERSHIP``
- ``IPV6_MULTICAST_IF``
- ``IPV6_MULTICAST_HOPS``
- ``IPV6_MULTICAST_LOOP``

``fcntl()``
^^^^^^^^^^^

The ``fcntl()`` function is a standard API for manipulating options related to a file descriptor. In ESP-IDF, the :doc:`/api-reference/storage/vfs` layer is used to implement this function.

When the file descriptor is a socket, only the following ``fcntl()`` values are supported:

- ``O_NONBLOCK`` to set or clear non-blocking I/O mode. Also supports ``O_NDELAY``, which is identical to ``O_NONBLOCK``.
- ``O_RDONLY``, ``O_WRONLY``, ``O_RDWR`` flags for different read or write modes. These flags can only be read using ``F_GETFL``, and cannot be set using ``F_SETFL``. A TCP socket returns a different mode depending on whether the connection has been closed at either end or is still open at both ends. UDP sockets always return ``O_RDWR``.

``ioctl()``
^^^^^^^^^^^

The ``ioctl()`` function provides a semi-standard way to access some internal features of the TCP/IP stack. In ESP-IDF, the :doc:`/api-reference/storage/vfs` layer is used to implement this function.

When the file descriptor is a socket, only the following ``ioctl()`` values are supported:

- ``FIONREAD`` returns the number of bytes of the pending data already received in the socket's network buffer.
- ``FIONBIO`` is an alternative way to set/clear non-blocking I/O status for a socket, equivalent to ``fcntl(fd, F_SETFL, O_NONBLOCK, ...)``.

Netconn API
-----------

lwIP supports two lower-level APIs as well as the BSD Sockets API: the Netconn API and the Raw API.

The lwIP Raw API is designed for single-threaded devices and is not supported in ESP-IDF.

The Netconn API is used to implement the BSD Sockets API inside lwIP, and it can also be called directly from ESP-IDF apps. This API has lower resource usage than the BSD Sockets API. In particular, it can send and receive data without firstly copying it into internal lwIP buffers.

.. important::

  Espressif does not test the Netconn API in ESP-IDF. As such, this functionality is **enabled but not supported**. Some functionality may only work correctly when used from the BSD Sockets API.

For more information about the Netconn API, consult `lwip/lwip/src/include/lwip/api.h <http://www.nongnu.org/lwip/2_0_x/api_8h.html>`_ and `part of the **unofficial** lwIP Application Developers Manual <https://lwip.fandom.com/wiki/Netconn_API>`_.

lwIP FreeRTOS Task
------------------

lwIP creates a dedicated TCP/IP FreeRTOS task to handle socket API requests from other tasks.

A number of configuration items are available to modify the task and the queues (mailboxes) used to send data to/from the TCP/IP task:

- :ref:`CONFIG_LWIP_TCPIP_RECVMBOX_SIZE`
- :ref:`CONFIG_LWIP_TCPIP_TASK_STACK_SIZE`
- :ref:`CONFIG_LWIP_TCPIP_TASK_AFFINITY`

IPv6 Support
------------

Both IPv4 and IPv6 are supported in a dual-stack configuration and are enabled by default. Both IPv6 and IPv4 may be disabled separately if they are not needed, see :ref:`lwip-ram-usage`.

IPv6 support is limited to **Stateless Autoconfiguration** only. **Stateful configuration** is not supported in ESP-IDF, nor in upstream lwIP.

IPv6 Address configuration is defined by means of these protocols or services:

- **SLAAC** IPv6 Stateless Address Autoconfiguration (RFC-2462)
- **DHCPv6** Dynamic Host Configuration Protocol for IPv6 (RFC-8415)

None of these two types of address configuration is enabled by default, so the device uses only Link Local addresses or statically-defined addresses.

.. _lwip-ivp6-autoconfig:

Stateless Autoconfiguration Process
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To enable address autoconfiguration using the Router Advertisement protocol, please enable:

- :ref:`CONFIG_LWIP_IPV6_AUTOCONFIG`

This configuration option enables IPv6 autoconfiguration for all network interfaces, which differs from the upstream lwIP behavior, where the autoconfiguration needs to be explicitly enabled for each ``netif`` with ``netif->ip6_autoconfig_enabled=1``.

.. _lwip-ivp6-dhcp6:

DHCPv6
^^^^^^

DHCPv6 in lwIP is very simple and supports only stateless configuration. It could be enabled using:

- :ref:`CONFIG_LWIP_IPV6_DHCP6`

Since the DHCPv6 works only in its stateless configuration, the :ref:`lwip-ivp6-autoconfig` has to be enabled as well via :ref:`CONFIG_LWIP_IPV6_AUTOCONFIG`.

Moreover, the DHCPv6 needs to be explicitly enabled from the application code using:

.. code-block::

    dhcp6_enable_stateless(netif);

DNS Servers in IPv6 Autoconfiguration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to autoconfigure DNS server(s), especially in IPv6-only networks, we have these two options:

- Recursive Domain Name System (DNS): this belongs to the Neighbor Discovery Protocol (NDP) and uses :ref:`lwip-ivp6-autoconfig`.

  The number of servers must be set :ref:`CONFIG_LWIP_IPV6_RDNSS_MAX_DNS_SERVERS`, this option is disabled by default, i.e., set to 0.

- DHCPv6 stateless configuration, uses :ref:`lwip-ivp6-dhcp6` to configure DNS servers. Note that this configuration assumes IPv6 Router Advertisement Flags (RFC-5175) to be set to

    - Managed Address Configuration Flag = 0
    - Other Configuration Flag = 1

ESP-lwIP Custom Modifications
-----------------------------

Additions
^^^^^^^^^

The following code is added, which is not present in the upstream lwIP release:

Thread-Safe Sockets
+++++++++++++++++++

It is possible to ``close()`` a socket from a different thread than the one that created it. The ``close()`` call blocks, until any function calls currently using that socket from other tasks have returned.

It is, however, not possible to delete a task while it is actively waiting on ``select()`` or ``poll()`` APIs. It is always necessary that these APIs exit before destroying the task, as this might corrupt internal structures and cause subsequent crashes of the lwIP. These APIs allocate globally referenced callback pointers on the stack so that when the task gets destroyed before unrolling the stack, the lwIP could still hold pointers to the deleted stack.

On-Demand Timers
++++++++++++++++

lwIP IGMP and MLD6 feature both initialize a timer in order to trigger timeout events at certain times.

The default lwIP implementation is to have these timers enabled all the time, even if no timeout events are active. This increases CPU usage and power consumption when using automatic Light-sleep mode. ``ESP-lwIP`` default behavior is to set each timer ``on demand``, so it is only enabled when an event is pending.

To return to the default lwIP behavior, which is always-on timers, disable :ref:`CONFIG_LWIP_TIMERS_ONDEMAND`.

lwIP Timers API
+++++++++++++++

When not using Wi-Fi, the lwIP timer can be turned off via the API to reduce power consumption.

The following API functions are supported. For full details, see :component_file:`lwip/lwip/src/include/lwip/timeouts.h`.

- ``sys_timeouts_init()``
- ``sys_timeouts_deinit()``

Additional Socket Options
+++++++++++++++++++++++++

- Some standard IPV4 and IPV6 multicast socket options are implemented, see `Socket Options`_.

- Possible to set IPV6-only UDP and TCP sockets with ``IPV6_V6ONLY`` socket option, while normal lwIP is TCP-only.

IP Layer Features
+++++++++++++++++

- IPV4-source-based routing implementation is different

- IPV4-mapped IPV6 addresses are supported

NAPT and Port Forwarding
++++++++++++++++++++++++

IPV4 network address port translation (NAPT) and port forwarding are supported. However, the enabling of NAPT is limited to a single interface.

- To use NAPT for forwarding packets between two interfaces, it needs to be enabled on the interface connecting to the target network. For example, to enable internet access for Ethernet traffic through the Wi-Fi interface, NAPT must be enabled on the Ethernet interface.
- Usage of NAPT is demonstrated in :example:`network/vlan_support`.

Default lwIP Hooks
++++++++++++++++++

ESP-IDF port layer provides a default hook file, which is included in the lwIP build process. This file is located at :component_file:`lwip/port/include/lwip_default_hooks.h` and defines several hooks that implement default ESP-IDF behavior of lwIP stack. These hooks could be further modified by choosing one of these options:

- *None*: No hook is declared.
- *Default*: Default IDF implementation is provided (declared as ``weak`` in most cases and can be overridden).
- *Custom*: Only the hook declaration is provided, and the application must implement it.

**DHCP Extra Option Hook**

ESP-IDF allows applications to define a hook for handling extra DHCP options. This can be useful for implementing custom DHCP-based behaviors, such as retrieving specific vendor options. To enable this feature, configure :ref:`CONFIG_LWIP_HOOK_DHCP_EXTRA_OPTION` either to **Default** (``weak`` implementation is provided which could be replaced by custom implementation) or **Custom** (you will have to implement the hook and define its link dependency to lwip)

**Example Usage**

An application can define the following function to handle a specific DHCP option (e.g., captive portal URI):

.. code-block::

    #include "esp_netif.h"
    #include "lwip/dhcp.h"

    void lwip_dhcp_on_extra_option(struct dhcp *dhcp, uint8_t state,
                                   uint8_t option, uint8_t len,
                                   struct pbuf* p, uint16_t offset)
    {
        if (option == ESP_NETIF_CAPTIVEPORTAL_URI) {
            char *uri = (char *)p->payload + offset;
            ESP_LOGI(TAG, "Captive Portal URI: %s", uri);
        }
    }

**Other Default Hooks**

ESP-IDF provides additional lwIP hooks that can be overridden. These include:

- TCP ISN Hook (:ref:`CONFIG_LWIP_HOOK_TCP_ISN`): Allows custom randomization of TCP Initial Sequence Numbers (ESP-IDF provided implementation is the default option, set to *Custom* to provide custom implementation or *None* to use lwIP implementation).
- IPv6 Route Hook (:ref:`CONFIG_LWIP_HOOK_IP6_ROUTE`): Enables custom route selection for IPv6 packets (No hook by default, use *Default* or *Custom* to override).
- IPv6 Get Gateway Hook (:ref:`CONFIG_LWIP_HOOK_ND6_GET_GW`): Enables defining a custom gateway selection logic (No hook by default, use *Default* or *Custom* to override).
- IPv6 Source Address Selection Hook (:ref:`CONFIG_LWIP_HOOK_IP6_SELECT_SRC_ADDR`): Allows customization of source address selection (No hook by default, use *Default* or *Custom* to override).
- Netconn External Resolve Hook (:ref:`CONFIG_LWIP_HOOK_NETCONN_EXTERNAL_RESOLVE`): Allows overriding the DNS resolution logic for network connections (No hook by default, use *Default* or *Custom* to override).
- DNS External Resolve Hook (:ref:`CONFIG_LWIP_HOOK_DNS_EXTERNAL_RESOLVE`): Provides a hook for custom DNS resolution logic with callbacks (No hook by default, but could be selected by an external component to prefer the custom option; use *Default* or *Custom* to override).
- IPv6 Packet Input Hook (:ref:`CONFIG_LWIP_HOOK_IP6_INPUT`): Provides filtering or modification of incoming IPv6 packets (ESP-IDF provided ``weak`` implementation is the default option; use *Custom* or provide a strong definition to override the *Default* option; choose *None* to disable IPv6 packet input filtering)

Each of these hooks can be configured in menuconfig, allowing selection of default, custom, or no implementation.

.. _lwip-custom-hooks:

Customized lwIP Hooks
+++++++++++++++++++++

The original lwIP supports implementing custom compile-time modifications via ``LWIP_HOOK_FILENAME``. This file is already used by the ESP-IDF port layer, but ESP-IDF users could still include and implement any custom additions via a header file defined by the macro ``ESP_IDF_LWIP_HOOK_FILENAME``. Here is an example of adding a custom hook file to the build process, and the hook is called ``my_hook.h``, located in the project's ``main`` folder:

.. code-block:: cmake

   idf_component_get_property(lwip lwip COMPONENT_LIB)
   target_compile_options(${lwip} PRIVATE "-I${PROJECT_DIR}/main")
   target_compile_definitions(${lwip} PRIVATE "-DESP_IDF_LWIP_HOOK_FILENAME=\"my_hook.h\"")

Customized lwIP Options From ESP-IDF Build System
++++++++++++++++++++++++++++++++++++++++++++++++++

The most common lwIP options are configurable through the component configuration menu. However, certain definitions need to be injected from the command line. The CMake function ``target_compile_definitions()`` can be employed to define macros, as illustrated below:

.. code-block:: cmake

   idf_component_get_property(lwip lwip COMPONENT_LIB)
   target_compile_definitions(${lwip} PRIVATE "-DETHARP_SUPPORT_VLAN=1")

This approach may not work for function-like macros, as there is no guarantee that the definition will be accepted by all compilers, although it is supported in GCC. To address this limitation, the ``add_definitions()`` function can be utilized to define the macro for the entire project, for example: ``add_definitions("-DFALLBACK_DNS_SERVER_ADDRESS(addr)=\"IP_ADDR4((addr), 8,8,8,8)\"")``.

Alternatively, you can define your function-like macro in a header file which will be pre-included as an lwIP hook file, see :ref:`lwip-custom-hooks`.

.. _lwip-limitations:

Limitations
^^^^^^^^^^^

lwIP in ESP-IDF supports thread safety in certain scenarios, but with limitations. It is possible to perform read, write, and close operations from different threads on the same socket simultaneously. However, performing multiple reads or multiple writes from more than one thread on the same socket at the same time is not supported. Applications that require simultaneous reads or writes from multiple threads on the same socket must implement additional synchronization mechanisms, such as locking around socket operations.

ESP-IDF additions to lwIP still suffer from the global DNS limitation, described in :ref:`lwip-dns-limitation`. To address this limitation from application code, the ``FALLBACK_DNS_SERVER_ADDRESS()`` macro can be utilized to define a global DNS fallback server accessible from all interfaces. Alternatively, you have the option to maintain per-interface DNS servers and reconfigure them whenever the default interface changes.

The number of IP addresses returned by network database APIs such as ``getaddrinfo()`` and ``gethostbyname()`` is restricted by the macro ``DNS_MAX_HOST_IP``. By default, the value of this macro is set to 1.

In the implementation of ``getaddrinfo()``, the canonical name is not available. Therefore, the ``ai_canonname`` field of the first returned ``addrinfo`` structure will always refer to the ``nodename`` argument or a string with the same contents.

The ``getaddrinfo()`` system call in lwIP within ESP-IDF has a limitation when using ``AF_UNSPEC``, as it defaults to returning only an IPv4 address in dual stack mode. This can cause issues in IPv6-only networks. To handle this, a workaround involves making two sequential calls to ``getaddrinfo()``: the first with ``AF_INET`` to query for IPv4 addresses, and the second with ``AF_INET6`` to retrieve IPv6 addresses. To provide a more robust solution, the custom ``esp_getaddrinfo()`` function has been added to the lwIP port layer to handle both IPv4 and IPv6 addresses when ``AF_UNSPEC`` is used. The :ref:`CONFIG_LWIP_USE_ESP_GETADDRINFO` option, available when both IPv4 and IPv6 are enabled, controls whether ``esp_getaddrinfo()`` or ``getaddrinfo()`` is used. It is disabled by default.

Calling ``send()`` or ``sendto()`` repeatedly on a UDP socket may eventually fail with ``errno`` equal to ``ENOMEM``. This failure occurs due to the limitations of buffer sizes in the lower-layer network interface drivers. If all driver transmit buffers are full, the UDP transmission will fail. For applications that transmit a high volume of UDP datagrams and aim to avoid any dropped datagrams by the sender, it is advisable to implement error code checking and employ a retransmission mechanism with a short delay.

.. only:: esp32

    Increasing the number of TX buffers in the :ref:`Wi-Fi <CONFIG_ESP_WIFI_TX_BUFFER>` or :ref:`Ethernet <CONFIG_ETH_DMA_TX_BUFFER_NUM>` project configuration as applicable may also help.

.. only:: not esp32 and SOC_WIFI_SUPPORTED

    Increasing the number of TX buffers in the :ref:`Wi-Fi <CONFIG_ESP_WIFI_TX_BUFFER>` project configuration may also help.

.. _lwip-performance:

Performance Optimization
------------------------

TCP/IP performance is a complex subject, and performance can be optimized toward multiple goals. The default settings of ESP-IDF are tuned for a compromise between throughput, latency, and moderate memory usage.

Maximum Throughput
^^^^^^^^^^^^^^^^^^

Espressif tests ESP-IDF TCP/IP throughput using the iperf test application: https://iperf.fr/, please refer to :ref:`improve-network-speed` for more details about the actual testing and using the optimized configuration.

.. important::

  Suggest applying changes a few at a time and checking the performance each time with a particular application workload.

- If a lot of tasks are competing for CPU time on the system, consider that the lwIP task has configurable CPU affinity (:ref:`CONFIG_LWIP_TCPIP_TASK_AFFINITY`) and runs at fixed priority (18, ``ESP_TASK_TCPIP_PRIO``). To optimize CPU utilization, consider assigning competing tasks to different cores or adjusting their priorities to lower values. For additional details on built-in task priorities, please refer to :ref:`built-in-task-priorities`.

- If using ``select()`` function with socket arguments only, disabling :ref:`CONFIG_VFS_SUPPORT_SELECT` will make ``select()`` calls faster.

- If there is enough free IRAM, select :ref:`CONFIG_LWIP_IRAM_OPTIMIZATION` and :ref:`CONFIG_LWIP_EXTRA_IRAM_OPTIMIZATION` to improve TX/RX throughput.

.. only:: SOC_WIFI_SUPPORTED

    If using a Wi-Fi network interface, please also refer to :ref:`wifi-buffer-usage`.

Minimum Latency
^^^^^^^^^^^^^^^

Except for increasing buffer sizes, most changes that increase throughput also decrease latency by reducing the amount of CPU time spent in lwIP functions.

- For TCP sockets, lwIP supports setting the standard ``TCP_NODELAY`` flag to disable Nagle's algorithm.

.. _lwip-ram-usage:

Minimum RAM Usage
^^^^^^^^^^^^^^^^^

Most lwIP RAM usage is on-demand, as RAM is allocated from the heap as needed. Therefore, changing lwIP settings to reduce RAM usage may not change RAM usage at idle, but can change it at peak.

- Reducing :ref:`CONFIG_LWIP_MAX_SOCKETS` reduces the maximum number of sockets in the system. This also causes TCP sockets in the ``WAIT_CLOSE`` state to be closed and recycled more rapidly when needed to open a new socket, further reducing peak RAM usage.
- Reducing :ref:`CONFIG_LWIP_TCPIP_RECVMBOX_SIZE`, :ref:`CONFIG_LWIP_TCP_RECVMBOX_SIZE` and :ref:`CONFIG_LWIP_UDP_RECVMBOX_SIZE` reduce RAM usage at the expense of throughput, depending on usage.
- Reducing :ref:`CONFIG_LWIP_TCP_ACCEPTMBOX_SIZE` reduce RAM usage by limiting concurrent accepted connections.
- Reducing :ref:`CONFIG_LWIP_TCP_MSL` and :ref:`CONFIG_LWIP_TCP_FIN_WAIT_TIMEOUT` reduces the maximum segment lifetime in the system. This also causes TCP sockets in the ``TIME_WAIT`` and ``FIN_WAIT_2`` states to be closed and recycled more rapidly.
- Disabling :ref:`CONFIG_LWIP_IPV6` can save about 39 KB for firmware size and 2 KB RAM when the system is powered up and 7 KB RAM when the TCP/IP stack is running. If there is no requirement for supporting IPV6, it can be disabled to save flash and RAM footprint.
- Disabling :ref:`CONFIG_LWIP_IPV4` can save about 26 KB of firmware size and 600 B RAM on power up and 6 KB RAM when the TCP/IP stack is running. If the local network supports IPv6-only configuration, IPv4 can be disabled to save flash and RAM footprint.

.. only:: SOC_WIFI_SUPPORTED

    If using Wi-Fi, please also refer to :ref:`wifi-buffer-usage`.


Peak Buffer Usage
+++++++++++++++++

The peak heap memory that lwIP consumes is the **theoretically-maximum memory** that the lwIP driver consumes. Generally, the peak heap memory that lwIP consumes depends on:

 - the memory required to create a UDP connection: ``lwip_udp_conn``
 - the memory required to create a TCP connection: ``lwip_tcp_conn``
 - the number of UDP connections that the application has: ``lwip_udp_con_num``
 - the number of TCP connections that the application has: ``lwip_tcp_con_num``
 - the TCP TX window size: ``lwip_tcp_tx_win_size``
 - the TCP RX window size: ``lwip_tcp_rx_win_size``

**So, the peak heap memory that the lwIP consumes can be calculated with the following formula:**
  lwip_dynamic_peek_memory =  (lwip_udp_con_num * lwip_udp_conn)  + (lwip_tcp_con_num * (lwip_tcp_tx_win_size + lwip_tcp_rx_win_size + lwip_tcp_conn))

Some TCP-based applications need only one TCP connection. However, they may choose to close this TCP connection and create a new one when an error occurs (e.g., a sending failure). This may result in multiple TCP connections existing in the system simultaneously, because it may take a long time for a TCP connection to close, according to the TCP state machine, refer to RFC793.


.. _lwIP lightweight TCP/IP stack: https://savannah.nongnu.org/projects/lwip/
.. _esp-lwip: https://github.com/espressif/esp-lwip
````

## File: docs/en/api-guides/memory-types.rst
````
.. _memory-layout:

Memory Types
------------

:link_to_translation:`zh_CN:[中文]`

{IDF_TARGET_NAME} chip has multiple memory types and flexible memory mapping features. This section describes how ESP-IDF uses these features by default.

ESP-IDF distinguishes between instruction memory bus (IRAM, IROM, RTC FAST memory) and data memory bus (DRAM, DROM). Instruction memory is executable, and can only be read or written via 4-byte aligned words. Data memory is not executable and can be accessed via individual byte operations. For more information about the different memory buses consult the *{IDF_TARGET_NAME} Technical Reference Manual* > *System and Memory*  [`PDF <{IDF_TARGET_TRM_EN_URL}#sysmem>`__].

.. _dram:

DRAM (Data RAM)
^^^^^^^^^^^^^^^

Non-constant static data (.data) and zero-initialized data (.bss) is placed by the linker into Internal SRAM as data memory. The remaining space in this region is used for the runtime heap.

.. only:: SOC_SPIRAM_SUPPORTED

   By applying the ``EXT_RAM_BSS_ATTR`` macro, zero-initialized data can also be placed into external RAM. To use this macro, the :ref:`CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` needs to be enabled. See :ref:`external_ram_config_bss`.

.. only:: esp32

   The available size of the internal DRAM region is reduced by 64 KB (by shifting start address to ``0x3FFC0000``) if Bluetooth stack is used. Length of this region is also reduced by 16 KB or 32 KB if trace memory is used. Due to some memory fragmentation issues caused by ROM, it is also not possible to use all available DRAM for static allocations - however the remaining DRAM is still available as heap at runtime.

   .. note::

    There is 520 KB of available SRAM (320 KB of DRAM and 200 KB of IRAM) on the ESP32. However, due to a technical limitation, the maximum statically allocated DRAM usage is 160 KB. The remaining 160 KB (for a total of 320 KB of DRAM) can only be allocated at runtime as heap.

.. only:: not esp32

   .. note::

    The maximum statically allocated DRAM size is reduced by the :ref:`iram` size of the compiled application. The available heap memory at runtime is reduced by the total static IRAM and DRAM usage of the application.

Constant data may also be placed into DRAM, for example if it is used in an non-flash-safe ISR (see explanation under :ref:`how-to-place-code-in-iram`).


"noinit" DRAM
=============

The macro ``__NOINIT_ATTR`` can be used as attribute to place data into ``.noinit`` section. The values placed into this section will not be initialized at startup and should keep its value after software restart.

.. only:: SOC_SPIRAM_SUPPORTED

    By applying the ``EXT_RAM_NOINIT_ATTR`` macro, non-initialized value could also be placed in external RAM. To do this, the :ref:`CONFIG_SPIRAM_ALLOW_NOINIT_SEG_EXTERNAL_MEMORY` needs to be enabled. See :ref:`external_ram_config_noinit`. If the :ref:`CONFIG_SPIRAM_ALLOW_NOINIT_SEG_EXTERNAL_MEMORY` is not enabled, ``EXT_RAM_NOINIT_ATTR`` will behave just as ``__NOINIT_ATTR``, it will make data to be placed into ``.noinit`` segment in internal RAM.

Example::

    __NOINIT_ATTR uint32_t noinit_data;


.. _iram:

IRAM (Instruction RAM)
^^^^^^^^^^^^^^^^^^^^^^

.. only:: esp32

    ESP-IDF allocates part of the Internal SRAM0 region for instruction RAM. The region is defined in *{IDF_TARGET_NAME} Technical Reference Manual* > *System and Memory* > *Embedded Memory* [`PDF <{IDF_TARGET_TRM_EN_URL}#sysmem>`__]. Except for the first 64 KB block which is used for PRO and APP MMU caches, the rest of this memory range (i.e., from ``0x40080000`` to ``0x400A0000``) is used to store parts of the application which need to run from RAM.

.. only:: esp32s2

    ESP-IDF allocates part of the Internal SRAM region for instruction RAM. The region is defined in *{IDF_TARGET_NAME} Technical Reference Manual* > *System and Memory* > *Internal Memory* [`PDF <{IDF_TARGET_TRM_EN_URL}#sysmem>`__]. Except for the first block (up to 32 KB) which is used for MMU cache, the rest of this memory range is used to store parts of application which need to run from RAM.

.. only:: not esp32

    .. note::

        Any internal SRAM which is not used for Instruction RAM will be made available as :ref:`dram` for static data and dynamic allocation (heap).


When to Place Code in IRAM
================================

Cases when parts of the application should be placed into IRAM:

- Interrupt handlers must be placed into IRAM if ``ESP_INTR_FLAG_IRAM`` is used when registering the interrupt handler. For more information, see :ref:`iram-safe-interrupt-handlers`.

- Some timing critical code may be placed into IRAM to reduce the penalty associated with loading the code from flash. {IDF_TARGET_NAME} reads code and data from flash via the MMU cache. In some cases, placing a function into IRAM may reduce delays caused by a cache miss and significantly improve that function's performance.


.. _how-to-place-code-in-iram:

How to Place Code in IRAM
=========================

Some code is automatically placed into the IRAM region using the linker script.

If some specific application code needs to be placed into IRAM, it can be done by using the :doc:`linker-script-generation` feature and adding a linker script fragment file to your component that targets at the entire source files or functions with the ``noflash`` placement. See the :doc:`linker-script-generation` docs for more information.

Alternatively, it is possible to specify IRAM placement in the source code using the ``IRAM_ATTR`` macro::

    #include "esp_attr.h"

    void IRAM_ATTR gpio_isr_handler(void* arg)
    {
        // ...
    }

There are some possible issues with placement in IRAM, that may cause problems with IRAM-safe interrupt handlers:

* Strings or constants inside an ``IRAM_ATTR`` function may not be placed in RAM automatically. It is possible to use ``DRAM_ATTR`` attributes to mark these, or using the linker script method will cause these to be automatically placed correctly.

  .. code-block:: c

    void IRAM_ATTR gpio_isr_handler(void* arg)
    {
       const static DRAM_ATTR uint8_t INDEX_DATA[] = { 45, 33, 12, 0 };
       const static char *MSG = DRAM_STR("I am a string stored in RAM");
    }

Note that knowing which data should be marked with ``DRAM_ATTR`` can be hard, the compiler will sometimes recognize that a variable or expression is constant (even if it is not marked ``const``) and optimize it into flash, unless it is marked with ``DRAM_ATTR``.

* GCC optimizations that automatically generate jump tables or switch/case lookup tables place these tables in flash. IDF by default builds all files with ``-fno-jump-tables -fno-tree-switch-conversion`` flags to avoid this.

Jump table optimizations can be re-enabled for individual source files that do not need to be placed in IRAM. For instructions on how to add the ``-fno-jump-tables -fno-tree-switch-conversion`` options when compiling individual source files, see :ref:`component_build_control`.


.. _irom:

IROM (Code Executed from flash)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If a function is not explicitly placed into :ref:`iram` or RTC memory, it is placed into flash. As IRAM is limited, most of an application's binary code must be placed into IROM instead.

.. only:: esp32

    The mechanism by which Flash MMU is used to allow code execution from flash is described in *{IDF_TARGET_NAME} Technical Reference Manual* > *Memory Management and Protection Units (MMU, MPU)* [`PDF <{IDF_TARGET_TRM_EN_URL}#mpummu>`__].

During :doc:`startup`, the bootloader (which runs from IRAM) configures the MMU flash cache to map the app's instruction code region to the instruction space. Flash accessed via the MMU is cached using some internal SRAM and accessing cached flash data is as fast as accessing other types of internal memory.


.. _drom:

DROM (Data Stored in flash)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. highlight:: c

By default, constant data is placed by the linker into a region mapped to the MMU flash cache. This is the same as the :ref:`irom` section, but is for read-only data not executable code.

The only constant data not placed into this memory type by default are literal constants which are embedded by the compiler into application code. These are placed as the surrounding function's executable instructions.

The ``DRAM_ATTR`` attribute can be used to force constants from DROM into the :ref:`dram` section (see above).

.. only:: SOC_RTC_SLOW_MEM_SUPPORTED

    RTC Slow Memory
    ^^^^^^^^^^^^^^^

    .. only:: ESP_ROM_SUPPORT_DEEP_SLEEP_WAKEUP_STUB

        Global and static variables used by code which runs from RTC memory must be placed into RTC Slow memory. For example :doc:`deep sleep <deep-sleep-stub>` variables can be placed here instead of RTC FAST memory, or code and variables accessed by the :doc:`/api-reference/system/ulp`.

    The attribute macro named ``RTC_NOINIT_ATTR`` can be used to place data into this type of memory. The values placed into this section keep their value after waking from deep sleep.

    Example::

        RTC_NOINIT_ATTR uint32_t rtc_noinit_data;


.. only:: SOC_RTC_FAST_MEM_SUPPORTED

    RTC FAST Memory
    ^^^^^^^^^^^^^^^

    .. only:: esp32c6 or esp32h2

        .. note::

            On {IDF_TARGET_NAME} what was previously referred to as RTC memory has been renamed LP (low power) memory. You might see both terms being used interchangeably in IDF code, docs and the technical reference manual.

    .. only:: ESP_ROM_SUPPORT_DEEP_SLEEP_WAKEUP_STUB

        The same region of RTC FAST memory can be accessed as both instruction and data memory. Code which has to run after wake-up from deep sleep mode has to be placed into RTC memory. Please check detailed description in :doc:`deep sleep <deep-sleep-stub>` documentation.

    .. only:: esp32

        In single core mode (:ref:`CONFIG_FREERTOS_UNICORE`), remaining RTC FAST memory is added to the heap, unless the option :ref:`CONFIG_ESP_SYSTEM_ALLOW_RTC_FAST_MEM_AS_HEAP` is disabled. This memory can be used interchangeably with :ref:`DRAM`, but is slightly slower to access and not DMA-capable.

        This option is not available in dual core mode, because on {IDF_TARGET_NAME}, RTC FAST memory can only be accessed by the PRO CPU.

    .. only:: not esp32

        Remaining RTC FAST memory is added to the heap unless the option :ref:`CONFIG_ESP_SYSTEM_ALLOW_RTC_FAST_MEM_AS_HEAP` is disabled. This memory can be used interchangeably with :ref:`DRAM`, but is slightly slower to access.


.. only:: SOC_MEM_TCM_SUPPORTED

    SPM (Scratchpad Memory)
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    SPM (Scratchpad Memory) is a dedicated on-chip memory located near the processor core, offering deterministic access timing. SPM does not rely on hardware caching mechanisms; instead, its access is explicitly managed by software, ensuring predictable and stable latency. The access latency of SPM is configuration dependent. When parity check is enabled, the latency is 4 clock cycles and memory bandwidth is reduced. When parity check is disabled, the latency is 1 clock cycle. This type of memory is typically used to store critical code and data that are sensitive to access timing, making it suitable for real-time systems or embedded applications with strict performance and response time requirements.


DMA-Capable Requirement
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. highlight:: c

Most peripheral DMA controllers (e.g., SPI, sdmmc, etc.) have requirements that sending/receiving buffers should be placed in DRAM and word-aligned. We suggest to place DMA buffers in static variables rather than in the stack. Use macro ``DMA_ATTR`` to declare global/local static variables like::

    DMA_ATTR uint8_t buffer[]="I want to send something";

    void app_main()
    {
        // initialization code...
        spi_transaction_t temp = {
            .tx_buffer = buffer,
            .length = 8 * sizeof(buffer),
        };
        spi_device_transmit(spi, &temp);
        // other stuff
    }

Or::

    void app_main()
    {
        DMA_ATTR static uint8_t buffer[] = "I want to send something";
        // initialization code...
        spi_transaction_t temp = {
            .tx_buffer = buffer,
            .length = 8 * sizeof(buffer),
        };
        spi_device_transmit(spi, &temp);
        // other stuff
    }

It is also possible to allocate DMA-capable memory buffers dynamically by using the :ref:`MALLOC_CAP_DMA <dma-capable-memory>` capabilities flag.

DMA Buffer in the Stack
^^^^^^^^^^^^^^^^^^^^^^^

Placing DMA buffers in the stack is possible but discouraged. If doing so, pay attention to the following:

.. list::

    :SOC_SPIRAM_SUPPORTED: - Placing DRAM buffers on the stack is not recommended if the stack may be in PSRAM. If the stack of a task is placed in the PSRAM, several steps have to be taken as described in :doc:`external-ram`.
    - Use macro ``WORD_ALIGNED_ATTR`` in functions before variables to place them in proper positions like::

        void app_main()
        {
            uint8_t stuff;
            WORD_ALIGNED_ATTR uint8_t buffer[] = "I want to send something";   //or the buffer will be placed right after stuff.
            // initialization code...
            spi_transaction_t temp = {
                .tx_buffer = buffer,
                .length = 8 * sizeof(buffer),
            };
            spi_device_transmit(spi, &temp);
            // other stuff
        }
````

## File: docs/en/api-guides/openthread.rst
````
OpenThread
==========

:link_to_translation:`zh_CN:[中文]`

`OpenThread <https://github.com/openthread/openthread>`_ is an IP stack running on the 802.15.4 MAC layer which features mesh network and low power consumption.

Modes of the OpenThread Stack
-----------------------------

OpenThread can run under the following modes on Espressif chips:

Standalone Node
+++++++++++++++

The full OpenThread stack and the application layer run on the same chip. This mode is available on chips with 15.4 radio such as ESP32-H2, ESP32-C6 and ESP32-C5.

Radio Co-Processor (RCP)
++++++++++++++++++++++++

The chip is connected to another host running the OpenThread IP stack. It sends and receives 15.4 packets on behalf of the host. This mode is available on chips with 15.4 radio such as ESP32-H2 and ESP32-C6. The underlying transport between the chip and the host can be SPI or UART. For the sake of latency, we recommend using SPI as the underlying transport.

OpenThread Host
+++++++++++++++

For chips without a 15.4 radio, it can be connected to an RCP and run OpenThread under host mode. This mode enables OpenThread on Wi-Fi chips such as ESP32, ESP32-S2, ESP32-S3, and ESP32-C3. The following diagram shows how devices work under different modes:

.. blockdiag::
    :caption: OpenThread device modes
    :align: center

    blockdiag openthread-device-modes {

        # global attributes
        node_height = 90;
        node_width = 220;
        span_width = 100
        default_shape = roundedbox;
        default_group_color = none;

        # node labels
        HOST_NODE [label="OpenThread \nhost\n(ESP32)", fontsize=14];
        RCP [label="Radio \nCo-Processor\n(ESP32-H2)", fontsize=14];
        STANDALONE [label="Standalone \nnode\n (ESP32-H2)", fontsize=14];

        # node connections + labels
        RCP -> STANDALONE [label="15.4 radio", dir=both, style=dashed];

        # arrange nodes vertically
        group {
           orientation = portrait;
           HOST_NODE -> RCP [label="SPI", dir=both];
        }
    }


How to Write an OpenThread Application
--------------------------------------

The OpenThread :example:`openthread/ot_cli` example is a good place to start at. It demonstrates basic OpenThread initialization and simple socket-based server and client.

Before OpenThread Initialization
++++++++++++++++++++++++++++++++

- s1.1: The main task calls :cpp:func:`esp_vfs_eventfd_register` to initialize the eventfd virtual file system. The eventfd file system is used for task notification in the OpenThread driver.

- s1.2: The main task calls :cpp:func:`nvs_flash_init` to initialize the NVS where the Thread network data is stored.

- s1.3: **Optional**. The main task calls :cpp:func:`esp_netif_init` only when it wants to create the network interface for Thread.

- s1.4: The main task calls :cpp:func:`esp_event_loop_create` to create the system Event task and initialize an application event's callback function.

OpenThread Stack Initialization
+++++++++++++++++++++++++++++++

- s2.1: Call :cpp:func:`esp_openthread_init` to initialize the OpenThread stack.

OpenThread Network Interface Initialization
+++++++++++++++++++++++++++++++++++++++++++

The whole stage is **optional** and only required if the application wants to create the network interface for Thread.

- s3.1: Call :cpp:func:`esp_netif_new` with ``ESP_NETIF_DEFAULT_OPENTHREAD`` to create the interface.
- s3.2: Call :cpp:func:`esp_openthread_netif_glue_init` to create the OpenThread interface handlers.
- s3.3: Call :cpp:func:`esp_netif_attach` to attach the handlers to the interface.

The OpenThread Main Loop
++++++++++++++++++++++++

- s4.3: Call :cpp:func:`esp_openthread_launch_mainloop` to launch the OpenThread main loop. Note that this is a busy loop and does not return until the OpenThread stack is terminated.

Calling OpenThread APIs
++++++++++++++++++++++++

The OpenThread APIs are not thread-safe. When calling OpenThread APIs from other tasks, make sure to hold the lock with :cpp:func:`esp_openthread_lock_acquire` and release the lock with :cpp:func:`esp_openthread_lock_release` afterwards.

Deinitialization
++++++++++++++++

The following steps are required to deinitialize the OpenThread stack:

- Call :cpp:func:`esp_netif_destroy` and :cpp:func:`esp_openthread_netif_glue_deinit` to deinitialize the OpenThread network interface if you have created one.
- Call :cpp:func:`esp_openthread_deinit` to deinitialize the OpenThread stack.


OpenThread Macro Definitions
----------------------------

In the OpenThread protocol stack, defining macros to enable features and configure parameters is a common practice. Users can define macro values to enable or disable specific features and adjust parameters. ESP provides the following methods for defining OpenThread macros:

- Using configuration menu (``menuconfig``): Some macros are mapped to Kconfig files and can be configured through ``idf.py menuconfig → Component config → OpenThread``. This allows enabling or disabling features and setting related parameters.
- Using user-defined header files: Users can create a custom header file and enable it via ``idf.py menuconfig → Component config → OpenThread → Thread Extended Features → Use a header file defined by customer``. The priority of the custom header file is second only to the ``menuconfig``.
- Using ``openthread-core-esp32x-xxx-config.h`` for configuration: Some macros have default values set in the OpenThread private header files. These cannot currently be modified through the ``menuconfig``, but can be modified via user-defined header files.
- Using OpenThread stack default configurations: Other macros are assigned default values when defined in the OpenThread stack.

.. note::

    The priority of the above configuration methods, from highest to lowest, is as follows: Configuration Menu → User-defined Header File → openthread-core-esp32x-xxx-config.h → OpenThread Stack Default Configuration

The OpenThread Border Router
----------------------------

The OpenThread border router connects the Thread network with other IP networks. It provides IPv6 connectivity, service registration, and commission functionality.

To launch an OpenThread border router on an ESP chip, you need to connect an RCP to a Wi-Fi capable chip such as ESP32.

Calling :cpp:func:`esp_openthread_border_router_init` during the initialization launches all the border routing functionalities.

You may refer to the :example:`openthread/ot_br` example and the README for further border router details.
````

## File: docs/en/api-guides/partition-tables.rst
````
Partition Tables
================

:link_to_translation:`zh_CN:[中文]`

Overview
--------

A single {IDF_TARGET_NAME}'s flash can contain multiple apps, as well as many different kinds of data (calibration data, filesystems, parameter storage, etc). For this reason a partition table is flashed to (:ref:`default offset <CONFIG_PARTITION_TABLE_OFFSET>`) 0x8000 in the flash.

The partition table length is 0xC00 bytes, as we allow a maximum of 95 entries. An MD5 checksum, used for checking the integrity of the partition table at runtime, is appended after the table data. Thus, the partition table occupies an entire flash sector, which size is 0x1000 (4 KB). As a result, any partition following it must be at least located at (:ref:`default offset <CONFIG_PARTITION_TABLE_OFFSET>`) + 0x1000.

Each entry in the partition table has a name (label), type (app, data, or something else), subtype and the offset in flash where the partition is loaded.

The simplest way to use the partition table is to open the project configuration menu (``idf.py menuconfig``) and choose one of the simple predefined partition tables under :ref:`CONFIG_PARTITION_TABLE_TYPE`:

* "Single factory app, no OTA"
* "Factory app, two OTA definitions"

In both cases the factory app is flashed at offset 0x10000. If you execute ``idf.py partition-table`` then it will print a summary of the partition table.

Built-in Partition Tables
-------------------------

Here is the summary printed for the "Single factory app, no OTA" configuration:

.. code-block:: none

    # ESP-IDF Partition Table
    # Name,   Type, SubType, Offset,  Size, Flags
    nvs,      data, nvs,     0x9000,  0x6000,
    phy_init, data, phy,     0xf000,  0x1000,
    factory,  app,  factory, 0x10000, 1M,

* At a 0x10000 (64 KB) offset in the flash is the app labelled "factory". The bootloader runs this app by default.
* There are also two data regions defined in the partition table for storing NVS library partition and PHY init data.

Here is the summary printed for the "Factory app, two OTA definitions" configuration:

.. code-block:: none

    # ESP-IDF Partition Table
    # Name,   Type, SubType, Offset,  Size, Flags
    nvs,      data, nvs,     0x9000,  0x4000,
    otadata,  data, ota,     0xd000,  0x2000,
    phy_init, data, phy,     0xf000,  0x1000,
    factory,  app,  factory, 0x10000,  1M,
    ota_0,    app,  ota_0,   0x110000, 1M,
    ota_1,    app,  ota_1,   0x210000, 1M,

* There are now three app partition definitions. The type of the factory app (at 0x10000) and the next two "OTA" apps are all set to "app", but their subtypes are different.
* There is also a new "otadata" slot, which holds the data for OTA updates. The bootloader consults this data in order to know which app to execute. If "ota data" is empty, it will execute the factory app.

Creating Custom Tables
----------------------

If you choose "Custom partition table CSV" in ``menuconfig``, then you can also enter the name of a CSV file (in the project directory) to use for your partition table. The CSV file can describe any number of definitions for the table you need.

The CSV format is the same format as printed in the summaries shown above. However, not all fields are required in the CSV. For example, here is the "input" CSV for the OTA partition table:

.. code-block:: none

    # Name,   Type, SubType,  Offset,   Size,  Flags
    nvs,      data, nvs,      0x9000,  0x4000
    otadata,  data, ota,      0xd000,  0x2000
    phy_init, data, phy,      0xf000,  0x1000
    factory,  app,  factory,  0x10000,  1M
    ota_0,    app,  ota_0,    ,         1M
    ota_1,    app,  ota_1,    ,         1M
    nvs_key,  data, nvs_keys, ,        0x1000

* Whitespace between fields is ignored, and so is any line starting with ``#`` (comments).
* Each non-comment line in the CSV file is a partition definition.
* If you change the value of :ref:`CONFIG_PARTITION_TABLE_OFFSET`, you should update any fixed ``Offset`` in your CSV file to avoid overlaps with the new partition table location. Alternatively, leaving the ``Offset`` field blank allows the ``gen_esp32part.py`` tool to automatically calculate the correct offset based on the current partition table offset and alignment requirements.

Here is an example of a CSV partition table that includes bootloader and partition table partitions:

.. code-block:: none

    # ESP-IDF Partition Table
    # Name,           Type,            SubType,  Offset,  Size,     Flags
    bootloader,       bootloader,      primary,  N/A,     N/A,
    partition_table,  partition_table, primary,  N/A,     N/A,
    nvs,              data,            nvs,      ,        0x6000,
    phy_init,         data,            phy,      ,        0x1000,
    factory,          app,             factory,  ,        1M,
    recoveryBloader,  bootloader,      recovery, N/A,     N/A,

The ``gen_esp32part.py`` tool will replace each ``N/A`` with appropriate values based on the selected Kconfig options: {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH} for the bootloader offset and :ref:`CONFIG_PARTITION_TABLE_OFFSET` for the partition table offset.

Name Field
~~~~~~~~~~

Name field can be any meaningful name. It is not significant to the {IDF_TARGET_NAME}. The maximum length of names is 16 bytes, including one null terminator. Names longer than the maximum length will be truncated.

Type Field
~~~~~~~~~~

Partition type field can be specified as a name or a number 0-254 (or as hex 0x00-0xFE). Types 0x00-0x3F are reserved for ESP-IDF core functions.

- ``app`` (0x00),
- ``data`` (0x01),
- ``bootloader`` (0x02). By default, this partition is not included in any CSV partition table files in ESP-IDF because it is not required and does not impact the system's functionality. It is only useful for the bootloader OTA updates and flash partitioning. Even if this partition is not present in the CSV file, it is still possible to perform the OTA.
- ``partition_table`` (0x03). By default, this partition also is not included in any CSV partition table files in ESP-IDF.
- 0x40-0xFE are reserved for **custom partition types**. If your app needs to store data in a format not already supported by ESP-IDF, then use a value from this range.

See :cpp:type:`esp_partition_type_t` for the enum definitions for ``app`` and ``data`` partitions.

If writing in C++ then specifying a application-defined partition type requires casting an integer to :cpp:type:`esp_partition_type_t` in order to use it with the :ref:`partition API<api-reference-partition-table>`. For example:

.. code-block:: none

    static const esp_partition_type_t APP_PARTITION_TYPE_A = (esp_partition_type_t)0x40;

The bootloader ignores any partition types other than ``app`` (0x00) and ``data`` (0x01).

SubType
~~~~~~~

{IDF_TARGET_ESP_PHY_REF:default = ":ref:`CONFIG_ESP_PHY_INIT_DATA_IN_PARTITION`", esp32p4, esp32c5, esp32c61="NOT UPDATED YET"}

The 8-bit SubType field is specific to a given partition type. ESP-IDF currently only specifies the meaning of the subtype field for ``app`` and ``data`` partition types.

See enum :cpp:type:`esp_partition_subtype_t` for the full list of subtypes defined by ESP-IDF, including the following:

.. only:: not esp32c6

  * When type is ``app``, the SubType field can be specified as ``factory`` (0x00), ``ota_0`` (0x10) ... ``ota_15`` (0x1F) or ``test`` (0x20).

.. only:: esp32c6

  * When type is ``app``, the SubType field can be specified as ``factory`` (0x00), ``ota_0`` (0x10) through ``ota_15`` (0x1F), or ``test`` (0x20). Additionally, if :doc:`ESP-TEE <../security/tee/tee>` functionality is enabled, two TEE-specific subtypes become available: ``tee_0`` (0x30) and ``tee_1`` (0x31).

    - ``factory`` (0x00) is the default app partition. The bootloader will execute the factory app unless there it sees a partition of type data/ota, in which case it reads this partition to determine which OTA image to boot.

        - OTA never updates the factory partition.
        - If you want to conserve flash usage in an OTA project, you can remove the factory partition and use ``ota_0`` instead.

    - ``ota_0`` (0x10) ... ``ota_15`` (0x1F) are the OTA app slots. When :doc:`OTA <../api-reference/system/ota>` is in use, the OTA data partition configures which app slot the bootloader should boot. When using OTA, an application should have at least two OTA application slots (``ota_0`` & ``ota_1``). Refer to the :doc:`OTA documentation <../api-reference/system/ota>` for more details.
    - ``test`` (0x20) is a reserved subtype for factory test procedures. It will be used as the fallback boot partition if no other valid app partition is found. It is also possible to configure the bootloader to read a GPIO input during each boot, and boot this partition if the GPIO is held low, see :ref:`bootloader_boot_from_test_firmware`.

    .. only:: esp32c6

      - ``tee_0`` (0x30) and ``tee_1`` (0x31) are the TEE app slots. When :doc:`TEE OTA <../security/tee/tee-ota>` is in use, the TEE OTA data partition configures which TEE app slot the bootloader should boot. When using TEE OTA, the partition table should have these two TEE app slots. Refer to the :doc:`TEE OTA documentation <../security/tee/tee-ota>` for more details.

* When type is ``bootloader``, the SubType field can be specified as:

    - ``primary`` (0x00). This is the 2nd stage bootloader, located at the {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH} address in flash memory. The tool automatically determines the appropriate size and offset for this subtype, so any size or offset specified for this subtype will be ignored. You can either leave these fields blank or use ``N/A`` as a placeholder.
    - ``ota`` (0x01). This is a temporary bootloader partition used by the bootloader OTA update functionality to download a new image. The tool ignores the size for this subtype, allowing you to leave it blank or use ``N/A``. You can only specify an offset, or leave it blank to have the tool calculate it based on the offsets of previously used partitions.
    - ``recovery`` (0x02). This is the recovery bootloader partition used for safely performing OTA updates to the bootloader. The ``gen_esp32part.py`` tool automatically determines the address and size for this partition, so you can leave these fields blank or use ``N/A`` as a placeholder. The address must match an eFuse field, which is defined through a Kconfig option. If the normal bootloader loading path fails, the first stage (ROM) bootloader will attempt to load the recovery partition at the address specified by the eFuse field.

    The size of the bootloader type is calculated by the ``gen_esp32part.py`` tool  based on the specified ``--offset`` (the partition table offset) and ``--primary-partition-offset`` arguments. Specifically, the bootloader size is defined as (:ref:`CONFIG_PARTITION_TABLE_OFFSET` - {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH}). This calculated size applies to all subtypes of the bootloader.

* When type is ``partition_table``, the SubType field can be specified as:

    - ``primary`` (0x00). This is the primary partition table, located at the :ref:`CONFIG_PARTITION_TABLE_OFFSET` address in flash memory. The tool automatically determines the appropriate size and offset for this subtype, so any size or offset specified for this subtype will be ignored. You can either leave these fields blank or use ``N/A`` as a placeholder.
    - ``ota`` (0x01). It is a temporary partition table partition used by the partition table OTA update functionality for downloading a new image. The tool ignores the size for this subtype, allowing you to leave it blank or use ``N/A``. You can specify an offset, or leave it blank, in which case the tool will calculate it based on the offsets of previously allocated partitions.

    The size for the ``partition_table`` type is fixed at ``0x1000`` and applies uniformly across all subtypes of ``partition_table``.

* When type is ``data``, the subtype field can be specified as ``ota`` (0x00), ``phy`` (0x01), ``nvs`` (0x02), nvs_keys (0x04), or a range of other component-specific subtypes (see :cpp:type:`subtype enum <esp_partition_subtype_t>`).

    - ``ota`` (0) is the :ref:`OTA data partition <ota_data_partition>` which stores information about the currently selected OTA app slot. This partition should be 0x2000 bytes in size. Refer to the :ref:`OTA documentation <ota_data_partition>` for more details.
    - ``phy`` (1) is for storing PHY initialisation data. This allows PHY to be configured per-device, instead of in firmware.

        - In the default configuration, the phy partition is not used and PHY initialisation data is compiled into the app itself. As such, this partition can be removed from the partition table to save space.
        - To load PHY data from this partition, open the project configuration menu (``idf.py menuconfig``) and enable {IDF_TARGET_ESP_PHY_REF} option. You will also need to flash your devices with phy init data as the esp-idf build system does not do this automatically.
    - ``nvs`` (2) is for the :doc:`Non-Volatile Storage (NVS) API <../api-reference/storage/nvs_flash>`.

        - NVS is used to store per-device PHY calibration data (different to initialisation data).

        .. only:: SOC_WIFI_SUPPORTED

            - NVS is used to store Wi-Fi data if the :doc:`esp_wifi_set_storage(WIFI_STORAGE_FLASH) <../api-reference/network/esp_wifi>` initialization function is used.

        - The NVS API can also be used for other application data.
        - It is strongly recommended that you include an NVS partition of at least 0x3000 bytes in your project.
        - If using NVS API to store a lot of data, increase the NVS partition size from the default 0x6000 bytes.
        - When NVS is used to store factory settings, it is recommended to keep these settings in a separate read-only NVS partition. The minimal size of a read-only NVS partition is 0x1000 bytes. See :ref:`read-only-nvs` for more details. ESP-IDF provides :doc:`NVS Partition Generator Utility </api-reference/storage/nvs_partition_gen>` to generate NVS partitions with factory settings and to flash them along with the application.
    - ``nvs_keys`` (4) is for the NVS key partition. See :doc:`Non-Volatile Storage (NVS) API <../api-reference/storage/nvs_flash>` for more details.

        - It is used to store NVS encryption keys when `NVS Encryption` feature is enabled.
        - The size of this partition should be 4096 bytes (minimum partition size).

    .. only:: esp32c6

            - ``tee_ota`` (0x90) is the :ref:`TEE OTA data partition <tee-ota-data-partition>` which stores information about the currently selected TEE OTA app slot. This partition should be 0x2000 bytes in size. Refer to the :doc:`TEE OTA documentation <../security/tee/tee-ota>` for more details.

    - There are other predefined data subtypes for data storage supported by ESP-IDF. These include:

        - ``coredump`` (0x03) is for storing core dumps while using a custom partition table CSV file. See :doc:`/api-guides/core_dump` for more details.
        - ``efuse`` (0x05) is for emulating eFuse bits using :ref:`virtual-efuses`.
        - ``undefined`` (0x06) is implicitly used for data partitions with unspecified (empty) subtype, but it is possible to explicitly mark them as undefined as well.
        - ``fat`` (0x81) is for :doc:`/api-reference/storage/fatfs`.
        - ``spiffs`` (0x82) is for :doc:`/api-reference/storage/spiffs`.
        - ``littlefs`` (0x83) is for `LittleFS filesystem <https://github.com/littlefs-project/littlefs>`_. See :example:`storage/littlefs` example for more details.

.. Comment: ``esphttpd`` (0x80) was not added to the list because there is no docs section for it and it is not clear whether user should use it explicitly.

    Other subtypes of ``data`` type are reserved for future ESP-IDF uses.

* If the partition type is any application-defined value (range 0x40-0xFE), then ``subtype`` field can be any value chosen by the application (range 0x00-0xFE).

    Note that when writing in C++, an application-defined subtype value requires casting to type :cpp:type:`esp_partition_subtype_t` in order to use it with the :ref:`partition API <api-reference-partition-table>`.

Extra Partition SubTypes
~~~~~~~~~~~~~~~~~~~~~~~~

A component can define a new partition subtype by setting the ``EXTRA_PARTITION_SUBTYPES`` property. This property is a CMake list, each entry of which is a comma separated string with ``<type>, <subtype>, <value>`` format. The build system uses this property to add extra subtypes and creates fields named ``ESP_PARTITION_SUBTYPE_<type>_<subtype>`` in :cpp:type:`esp_partition_subtype_t`. The project can use this subtype to define partitions in the partitions table CSV file and use the new fields in :cpp:type:`esp_partition_subtype_t`.

.. _partition-offset-and-size:

Offset & Size
~~~~~~~~~~~~~

.. list::

    - The offset represents the partition address in the SPI flash, which sector size is 0x1000 (4 KB). Thus, the offset must be a multiple of 4 KB.
    - Partitions with blank offsets in the CSV file will start after the previous partition, or after the partition table in the case of the first partition.
    - Partitions of type ``app`` have to be placed at offsets aligned to 0x10000 (64 KB). If you leave the offset field blank, ``gen_esp32part.py`` will automatically align the partition. If you specify an unaligned offset for an ``app`` partition, the tool will return an error.
    - Partitions of type ``app`` should have the size aligned to the flash sector size (4 KB). If you specify an unaligned size for an ``app`` partition, the tool will return an error.
    :SOC_SECURE_BOOT_V1: - If Secure Boot V1 is enabled, then the partition of type ``app`` needs to have size aligned to 0x10000 (64 KB) boundary.
    :SOC_SECURE_BOOT_V1: - The ``bootloader`` offset and size are not affected by the Secure Boot V1 option. Whether Secure Boot V1 is enabled or not, the bootloader remains the same size and does not include the secure digest, which is flashed at offset 0x0 in the flash and occupies one sector (4096 bytes).
    - Sizes and offsets can be specified as decimal numbers, hex numbers with the prefix 0x, or size multipliers K or M (1024 and 1024*1024 bytes).
    - For ``bootloader`` and ``partition_table`` types, specifying ``N/A`` for size and offset in the CSV file means that these values are automatically determined by the tool and cannot be manually defined. This requires setting the ``--offset`` and ``--primary-partition-offset`` arguments of ``gen_esp32part.py``.

.. note::

    If you want the partitions in the partition table to work relative to any placement (:ref:`CONFIG_PARTITION_TABLE_OFFSET`) of the table itself, leave the offset field (in CSV file) for all partitions blank. Similarly, if changing the partition table offset, then be aware that all blank partition offsets may change to match, and that any fixed offsets may now collide with the partition table (causing an error).

Flags
~~~~~

Two flags are currently supported, ``encrypted`` and ``readonly``:

    - If ``encrypted`` flag is set, the partition will be encrypted if :doc:`/security/flash-encryption` is enabled.

    .. note::

        The following type partitions will always be encrypted, regardless of whether this flag is set or not:

        .. list::

            - ``app``,
            - ``bootloader``,
            - ``partition_table``,
            - type ``data`` and subtype ``ota``,
            - type ``data`` and subtype ``nvs_keys``.

    - If ``readonly`` flag is set, the partition will be read-only. This flag is only supported for ``data`` type partitions except ``ota`` and ``coredump`` subtypes. This flag can help to protect against accidental writes to a partition that contains critical device-specific configuration data, e.g., factory data partition.

    .. note::

        Using C file I/O API to open a file (``fopen``) in any write mode (``w``, ``w+``, ``a``, ``a+``, ``r+``) will fail and return ``NULL``. Using ``open`` with any other flag than ``O_RDONLY`` will fail and return ``-1`` while ``errno`` global variable will be set to ``EROFS``. This is also true for any other POSIX syscall function performing write or erase operations. Opening a handle in read-write mode for NVS on a read-only partition will fail and return :c:macro:`ESP_ERR_NOT_ALLOWED` error code. Using a lower level API like ``esp_partition``, ``spi_flash``, etc. to write to a read-only partition will result in :c:macro:`ESP_ERR_NOT_ALLOWED` error code.

You can specify multiple flags by separating them with a colon. For example, ``encrypted:readonly``.

Generating Binary Partition Table
---------------------------------

The partition table which is flashed to the {IDF_TARGET_NAME} is in a binary format, not CSV. The tool :component_file:`partition_table/gen_esp32part.py` is used to convert between CSV and binary formats.

If you configure the partition table CSV name in the project configuration (``idf.py menuconfig``) and then build the project or run ``idf.py partition-table``, this conversion is done as part of the build process.

To convert CSV to Binary manually:

.. code-block:: none

    python gen_esp32part.py input_partitions.csv binary_partitions.bin

To convert binary format back to CSV manually:

.. code-block:: none

    python gen_esp32part.py binary_partitions.bin input_partitions.csv

To display the contents of a binary partition table on stdout (this is how the summaries displayed when running ``idf.py partition-table`` are generated:

.. code-block:: none

    python gen_esp32part.py binary_partitions.bin

Partition Size Checks
---------------------

The ESP-IDF build system will automatically check if generated binaries fit in the available partition space, and will fail with an error if a binary is too large.

Currently these checks are performed for the following binaries:

* Bootloader binary must fit in space before partition table (see :ref:`bootloader-size`).
* App binary should fit in at least one partition of type "app". If the app binary does not fit in any app partition, the build will fail. If it only fits in some of the app partitions, a warning is printed about this.

.. note::

    Although the build process will fail if the size check returns an error, the binary files are still generated and can be flashed (although they may not work if they are too large for the available space.)

MD5 Checksum
~~~~~~~~~~~~

The binary format of the partition table contains an MD5 checksum computed based on the partition table. This checksum is used for checking the integrity of the partition table during the boot.

.. only:: esp32

    The MD5 checksum generation can be disabled by the ``--disable-md5sum`` option of ``gen_esp32part.py`` or by the :ref:`CONFIG_PARTITION_TABLE_MD5` option. This is useful for example when one :ref:`uses a bootloader from ESP-IDF before v3.1 <CONFIG_APP_COMPATIBLE_PRE_V3_1_BOOTLOADERS>` which cannot process MD5 checksums and the boot fails with the error message ``invalid magic number 0xebeb``.

.. only:: not esp32

    The MD5 checksum generation can be disabled by the ``--disable-md5sum`` option of ``gen_esp32part.py`` or by the :ref:`CONFIG_PARTITION_TABLE_MD5` option.


Flashing the Partition Table
----------------------------

* ``idf.py partition-table-flash``: will flash the partition table with esptool.py.
* ``idf.py flash``: Will flash everything including the partition table.

A manual flashing command is also printed as part of ``idf.py partition-table`` output.

.. note::

    Note that updating the partition table does not erase data that may have been stored according to the old partition table. You can use ``idf.py erase-flash`` (or ``esptool.py erase_flash``) to erase the entire flash contents.


Partition Tool (``parttool.py``)
--------------------------------

The component `partition_table` provides a tool :component_file:`parttool.py <partition_table/parttool.py>` for performing partition-related operations on a target device. The following operations can be performed using the tool:

    - reading a partition and saving the contents to a file (read_partition)
    - writing the contents of a file to a partition (write_partition)
    - erasing a partition (erase_partition)
    - retrieving info such as name, offset, size and flag ("encrypted") of a given partition (get_partition_info)

The tool can either be imported and used from another Python script or invoked from shell script for users wanting to perform operation programmatically. This is facilitated by the tool's Python API and command-line interface, respectively.

Python API
~~~~~~~~~~~

Before anything else, make sure that the `parttool` module is imported.

.. code-block:: python

    import sys
    import os

    idf_path = os.environ["IDF_PATH"]  # get value of IDF_PATH from environment
    parttool_dir = os.path.join(idf_path, "components", "partition_table")  # parttool.py lives in $IDF_PATH/components/partition_table

    sys.path.append(parttool_dir)  # this enables Python to find parttool module
    from parttool import *  # import all names inside parttool module

The starting point for using the tool's Python API to do is create a `ParttoolTarget` object:

.. code-block:: python

    # Create a parttool.py target device connected on serial port /dev/ttyUSB1
    target = ParttoolTarget("/dev/ttyUSB1")

The created object can now be used to perform operations on the target device:

.. code-block:: python

    # Erase partition with name 'storage'
  target.erase_partition(PartitionName("storage"))

    # Read partition with type 'data' and subtype 'spiffs' and save to file 'spiffs.bin'
    target.read_partition(PartitionType("data", "spiffs"), "spiffs.bin")

    # Write to partition 'factory' the contents of a file named 'factory.bin'
    target.write_partition(PartitionName("factory"), "factory.bin")

    # Print the size of default boot partition
    storage = target.get_partition_info(PARTITION_BOOT_DEFAULT)
    print(storage.size)

The partition to operate on is specified using `PartitionName` or `PartitionType` or PARTITION_BOOT_DEFAULT. As the name implies, these can be used to refer to partitions of a particular name, type-subtype combination, or the default boot partition.

More information on the Python API is available in the docstrings for the tool.

Command-line Interface
~~~~~~~~~~~~~~~~~~~~~~

The command-line interface of `parttool.py` has the following structure:

.. code-block:: bash

    parttool.py [command-args] [subcommand] [subcommand-args]

    - command-args - These are arguments that are needed for executing the main command (parttool.py), mostly pertaining to the target device
    - subcommand - This is the operation to be performed
    - subcommand-args - These are arguments that are specific to the chosen operation

.. code-block:: bash

    # Erase partition with name 'storage'
    parttool.py --port "/dev/ttyUSB1" erase_partition --partition-name=storage

    # Read partition with type 'data' and subtype 'spiffs' and save to file 'spiffs.bin'
    parttool.py --port "/dev/ttyUSB1" read_partition --partition-type=data --partition-subtype=spiffs --output "spiffs.bin"

    # Write to partition 'factory' the contents of a file named 'factory.bin'
    parttool.py --port "/dev/ttyUSB1" write_partition --partition-name=factory --input "factory.bin"

    # Print the size of default boot partition
    parttool.py --port "/dev/ttyUSB1" get_partition_info --partition-boot-default --info size

.. note::

    If the device has already enabled ``Flash Encryption`` or ``Secure Boot``, attempting to use commands that modify the flash content, such as ``erase_partition`` or ``write_partition``, will result in an error. This error is generated by the erase command of ``esptool.py``, which is called first before writing. This error is done as a safety measure to prevent bricking your device.

    .. code-block:: none

        A fatal error occurred: Active security features detected, erasing flash is disabled as a safety measure. Use --force to override, please use with caution, otherwise it may brick your device!

    To work around this, you need use the ``--force`` flag with ``esptool.py``. Specifically, the ``parttool.py`` provides the ``--esptool-erase-args`` argument that help to pass this flag to ``esptool.py``.

    .. code-block:: bash

        # Erase partition with name 'storage'
        # If Flash Encryption or Secure Boot are enabled then add "--esptool-erase-args=force"
        parttool.py --port "/dev/ttyUSB1" --esptool-erase-args=force erase_partition --partition-name=storage

        # Write to partition 'factory' the contents of a file named 'factory.bin'
        # If Flash Encryption or Secure Boot are enabled then add "--esptool-erase-args=force"
        parttool.py --port "/dev/ttyUSB1" --esptool-erase-args=force write_partition --partition-name=factory --input "factory.bin"

More information can be obtained by specifying `--help` as argument:

.. code-block:: bash

    # Display possible subcommands and show main command argument descriptions
    parttool.py --help

    # Show descriptions for specific subcommand arguments
    parttool.py [subcommand] --help

.. _secure boot: security/secure-boot-v1.rst
````

## File: docs/en/api-guides/phy.rst
````
PHY
==================

:link_to_translation:`zh_CN:[中文]`

Multiple Antennas
--------------------------

Principles and Components of Multiple Antennas
+++++++++++++++++++++++++++++++++++++++++++++++

Multi-antenna functionality primarily involves routing signals from internal antenna modules to specific IO pins, controlling external antenna switches through IO pins to select designated antennas, supporting up to 16 antennas.

The components of multiple antennas can be depicted as following picture:

.. code-block:: none

                         ___________________________________________________________________________
                    ____|____                          _________                                    |
                   |PHY      |--antenna_select[0] --> |         |                                   |
                ___|Antenna 0|--antenna_select[1] --> |         |                                   |
               /   |Module   |--antenna_select[2] --> |         |                               ____|____
              /    |_________|--antenna_select[3] --> | IO MUX  |--GPIO[x0] --> ant_sel_in[0]--|         | --- antenna 0
    RX/TX ___/          |                             | And     |--GPIO[x1] --> ant_sel_in[1]--| Antenna | --- antenna 1
             \      ____|____                         | GPIO    |--GPIO[x2] --> ant_sel_in[2]--| Switch  | ...  ...
              \    |PHY      |--antenna_select[0] --> | Matrix  |--GPIO[x3] --> ant_sel_in[3]--|_________| --- antenna 15
               \___|Antenna 1|--antenna_select[1] --> |         |
                   |Module   |--antenna_select[2] --> |         |
                   |_________|--antenna_select[3] --> |_________|

{IDF_TARGET_NAME} Multiple antennas primarily consists of three parts: the PHY Antenna Module inside the chip, IO MUX and GPIO Matrix, and external antenna switches.

1. PHY Antenna Module:

    - Both antenna modules support operation in transmit (TX) or receive (RX) mode, and can be configured via software to select a particular module for transmission or reception.
    - Each antenna module supports outputting up to 4 antenna selection signals ``antenna_select[3:0]``, which can be configured by software and mapped to any IO pin individually.
    - When an antenna module is in operation, the logic level of the IO pin corresponds to the configured signal value.

2. IO MUX and GPIO Matrix:

    - Routes the internal 4-way antenna signals to specific IO pins.

3. External Antenna Switches:

    - Typically multi-way selectors, they choose the working antenna based on the logic level of the ``ant_sel_in[x]`` pin. For example, ``ant_sel_in[3:0]`` as ``0b1011`` selects antenna 11.

Steps for Multi-Antenna Usage
++++++++++++++++++++++++++++++

1. Determine the IO pins used for controlling antenna switching based on hardware circuit design and external antenna switches.

2. Configure antenna selection signals to output to specified IO pins.

    - API :cpp:func:`esp_phy_set_ant_gpio()` is used to configure ``antenna_selects[3:0]`` signals to connect with ``GPIO[x3:x0]``. If ``GPIO[x0]`` is connected to ``antenna_select[0]``, ``gpio_config->gpio_cfg[x0].gpio_select`` should be set to 1, and the value of ``gpio_config->gpio_cfg[x0].gpio_num`` should be ``GPIO[x0]``.

3. Configure internal antenna operation mode and output signals.

    - API :cpp:func:`esp_phy_set_ant()` is used to configure the use of internal antenna module 0 or 1 for transmission or reception, and to configure the output signal values when antenna module 0 or 1 is in operation.
    - ``ESP_PHY_ANT_MODE_AUTO`` mode is currently not recommended for use.

Multi-Antenna Configuration Reference Example
+++++++++++++++++++++++++++++++++++++++++++++++

Typically, the following steps can be performed to configure multi-antenna:

- Configure ``antenna_selects`` to connect with which GPIOs. For example, if four antennas are supported and GPIO20/GPIO21 are connected to ``antenna_select[0]/antenna_select[1]``, the configuration is as follows:

.. code-block:: c

    esp_phy_ant_gpio_config_t ant_gpio_config = {
        .gpio_cfg[0] = { .gpio_select = 1, .gpio_num = 20 },
        .gpio_cfg[1] = { .gpio_select = 1, .gpio_num = 21 }
    };

- Configure which antennas are enabled and how enabled antennas are used for receiving/sending data. For example, if antennas 1 and 3 are enabled, data reception needs to automatically select the better antenna, with antenna 1 set as the default antenna, and data transmission always selecting antenna 3. The configuration is as follows:

.. code-block:: c

    esp_phy_ant_config_t config = {
        .rx_ant_mode = ESP_PHY_ANT_MODE_AUTO,
        .rx_ant_default = ESP_PHY_ANT_ANT0,
        .tx_ant_mode = ESP_PHY_ANT_MODE_ANT1,
        .enabled_ant0 = 1,
        .enabled_ant1 = 3
    };

Notes
++++++++++++++++++++++++++

1. Different antenna switches may have invalid input values for ``ant_sel_in[3:0]``, meaning the number of antennas supported by {IDF_TARGET_NAME} via external antenna switches may be less than 16. For example, ESP32-WROOM-DA uses RTC6603SP as the antenna switch, supporting only 2 antennas. The two antenna selection input pins are active high and are connected to two GPIOs. ``0b01`` indicates antenna 0 is selected, ``0b10`` indicates antenna 1 is selected. Input values ``0b00`` and ``0b11`` are invalid.

2. Despite supporting up to 16 antennas, only a maximum of two antennas can be enabled simultaneously for sending and receiving data.

3. The use of ``ESP_PHY_ANT_MODE_AUTO`` mode is currently not recommended, primarily due to the following limitations:

    - For the antenna selection algorithm based on ``ESP_PHY_ANT_MODE_AUTO`` type for sending data, the antenna for sending data can only be set to ``ESP_PHY_ANT_MODE_AUTO`` when the antenna mode for receiving data is ``ESP_PHY_ANT_MODE_AUTO``.
    - When the receiving or sending antenna mode is configured as ``ESP_PHY_ANT_MODE_AUTO``, frequent antenna switching may occur if RF signal degradation is detected. Unstable RF signals can lead to frequent antenna switching, resulting in the overall RF performance not meeting expectations.

Recommended Scenarios for Using Multiple Antennas
++++++++++++++++++++++++++++++++++++++++++++++++++

1. Applications can either select specified antennas or implement their own antenna selection algorithms based on collected information, such as selecting antenna modes according to application-specific criteria. Refer to the IDF example :idf_file:`examples/phy/antenna/README.md` for designing antenna selection algorithms.

2. Configure antenna modes for both receiving and sending data as ``ESP_PHY_ANT_MODE_ANT0`` or ``ESP_PHY_ANT_MODE_ANT1``.

Application Examples
--------------------

.. only:: esp32c3 or esp32s3

    - :example:`phy/cert_test` demonstrates how to use the Certification Test APIs on {IDF_TARGET_NAME}, including project configuration as well as executing RF, Wi-Fi, and Bluetooth certification tests.

- :example:`phy/antenna` demonstrates how to use multi-antenna software switching for {IDF_TARGET_NAME}.
````

## File: docs/en/api-guides/reproducible-builds.rst
````
Reproducible Builds
===================

:link_to_translation:`zh_CN:[中文]`

Introduction
------------

ESP-IDF build system has support for `reproducible builds <https://reproducible-builds.org/docs/definition/>`_.

When reproducible builds are enabled, the application built with ESP-IDF does not depend on the build environment. Both the ``.elf`` file and ``.bin`` files of the application remains exactly the same, even if the following variables change:

- Directory where the project is located
- Directory where ESP-IDF is located (``IDF_PATH``)
- Build time
- Toolchain installation path

Reasons for Non-Reproducible Builds
-----------------------------------

There are several reasons why an application may depend on the build environment, even when the same source code and tools versions are used.

- In C code, ``__FILE__`` preprocessor macro is expanded to the full path of the source file.
- ``__DATE__`` and ``__TIME__`` preprocessor macros are expanded to compilation date and time.
- When the compiler generates object files, it adds sections with debug information. These sections help debuggers, like GDB, to locate the source code which corresponds to a particular location in the machine code. These sections typically contain paths of relevant source files. These paths may be absolute, and will include the path to ESP-IDF or to the project.

There are also other possible reasons, such as unstable order of inputs and non-determinism in the build system.

Enabling Reproducible Builds in ESP-IDF
---------------------------------------

Reproducible builds can be enabled in ESP-IDF using :ref:`CONFIG_APP_REPRODUCIBLE_BUILD` option.

This option is disabled by default. It can be enabled in ``menuconfig``.

The option may also be added into ``sdkconfig.defaults``. If adding the option into ``sdkconfig.defaults``, delete the ``sdkconfig`` file and run the build again. See :ref:`custom-sdkconfig-defaults` for more information.

How Reproducible Builds Are Achieved
------------------------------------

ESP-IDF achieves reproducible builds using the following measures:

- In ESP-IDF source code, ``__DATE__`` and ``__TIME__`` macros are not used when reproducible builds are enabled. Note, if the application source code uses these macros, the build will not be reproducible.
- ESP-IDF build system passes a set of ``-fmacro-prefix-map`` and ``-fdebug-prefix-map`` flags to replace base paths with placeholders:

    - Path to ESP-IDF is replaced with ``/IDF``
    - Path to the project is replaced with ``/IDF_PROJECT``
    - Path to the build directory is replaced with ``/IDF_BUILD``
    - Paths to components are replaced with ``/COMPONENT_NAME_DIR`` (where ``NAME`` is the name of the component)
    - Path to the toolchain is replaced with ``/TOOLCHAIN``

- Build date and time are not included into the :ref:`application  metadata structure <app-image-format-application-description>` and :ref:`bootloader metadata structure <image-format-bootloader-description>` if :ref:`CONFIG_APP_REPRODUCIBLE_BUILD` is enabled.
- ESP-IDF build system ensures that source file lists, component lists and other sequences are sorted before passing them to CMake. Various other parts of the build system, such as the linker script generator also perform sorting to ensure that same output is produced regardless of the environment.

.. _reproducible-builds-and-debugging:

Reproducible Builds and Debugging
---------------------------------

When reproducible builds are enabled, file names included in debug information sections are altered as shown in the previous section. Due to this fact, the debugger (GDB) is not able to locate the source files for the given code location.

This issue can be solved using GDB ``set substitute-path`` command. For example, by adding the following command to GDB init script, the altered paths can be reverted to the original ones.

.. code-block:: none

    set substitute-path /COMPONENT_FREERTOS_DIR /home/user/esp/esp-idf/components/freertos

ESP-IDF build system generates a file with the list of such ``set substitute-path`` commands automatically during the build process. The file is called ``prefix_map`` and is located in the project ``build/gdbinit`` directory.

When :ref:`idf.py gdb <jtag-debugging-with-idf-py>` is used to start debugging, this additional ``gdbinit`` file is automatically passed to GDB. When launching GDB manually or from IDE, please pass this additional ``gdbinit`` script to GDB using ``-x build/gdbinit/prefix_map`` argument.

Factors Which Still Affect Reproducible Builds
----------------------------------------------

Note that the built application still depends on:

- ESP-IDF version
- Versions of the build tools (CMake, Ninja) and the cross-compiler

:doc:`tools/idf-docker-image` can be used to ensure that these factors do not affect the build.
````

## File: docs/en/api-guides/RF_calibration.rst
````
RF Calibration
==============

:link_to_translation:`zh_CN:[中文]`

{IDF_TARGET_NAME} supports three RF calibration methods during RF initialization:

1. Partial calibration

2. Full calibration

3. No calibration

Partial Calibration
-------------------

During RF initialization, the partial calibration method is used by default for RF calibration. It is done based on the full calibration data which is stored in the NVS. To use this method, please go to ``menuconfig`` and enable :ref:`CONFIG_ESP_PHY_CALIBRATION_AND_DATA_STORAGE`.

Full Calibration
----------------

Full calibration is triggered in the following conditions:

1. NVS does not exist.

2. The NVS partition to store calibration data has been erased.

3. Hardware MAC address has changed.

4. PHY library version has changed.

5. The RF calibration data loaded from the NVS partition is broken.

Full calibration takes 100 ms longer than the partial calibration method. If boot duration is not of critical importance to the application, the full calibration method is recommended. To switch to the full calibration method, go to ``menuconfig`` and disable :ref:`CONFIG_ESP_PHY_CALIBRATION_AND_DATA_STORAGE`. If you use the default method of RF calibration, there are two ways to add the function of triggering full calibration as a last-resort remedy.

1. Erase the NVS partition if you do not mind all of the data stored in the NVS partition is erased. That is indeed the easiest way.

2. Call API :cpp:func:`esp_phy_erase_cal_data_in_nvs` before initializing Wi-Fi and Bluetooth®/Bluetooth Low Energy based on some conditions (e.g., an option provided in some diagnostic mode). In this case, only the PHY namespace of the NVS partition is erased.

No Calibration
---------------

The no calibration method is only used when the device wakes up from Deep-sleep mode.

PHY Initialization Data
-----------------------

The PHY initialization data is used for RF calibration. There are two ways to get the PHY initialization data.

One is to use the default initialization data which is located in the header file :idf_file:`components/esp_phy/{IDF_TARGET_PATH_NAME}/include/phy_init_data.h`. It is embedded into the application binary after compiling and then stored into read-only memory (DROM). To use the default initialization data, please go to ``menuconfig`` and disable :ref:`CONFIG_ESP_PHY_INIT_DATA_IN_PARTITION`.

An alternative is to store the initialization data in a PHY data partition. A PHY data partition is included in the default partition table. However, when using a custom partition table, please ensure that a PHY data partition (type: ``data``, subtype: ``phy``) is included in the custom partition table. Whether you are using a custom partition table or the default partition table, if initialization data is stored in a partition, it has to be flashed there, otherwise a runtime error occurs. If you want to use initialization data stored in a partition, go to ``menuconfig`` and enable the option :ref:`CONFIG_ESP_PHY_INIT_DATA_IN_PARTITION`.

API Reference
-------------

.. include-build-file:: inc/esp_phy_init.inc
.. include-build-file:: inc/esp_phy_cert_test.inc
````

## File: docs/en/api-guides/startup.rst
````
Application Startup Flow
========================

:link_to_translation:`zh_CN:[中文]`

This note explains various steps which happen before ``app_main`` function of an ESP-IDF application is called.

The high level view of startup process is as follows:

.. list::

    1. :ref:`first-stage-bootloader` loads the second stage bootloader image to RAM (IRAM & DRAM) from flash offset {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH}.

    2. :ref:`second-stage-bootloader` loads partition table and main app image from flash. Main app incorporates both RAM segments and read-only segments mapped via flash cache.

    :SOC_HP_CPU_HAS_MULTIPLE_CORES: 3. :ref:`application-startup` executes. At this point, the second CPU and RTOS scheduler are started, which then run the ``main_task``, leading to the execution of ``app_main``.

    :not SOC_HP_CPU_HAS_MULTIPLE_CORES: 3. :ref:`application-startup` executes. At this point, the RTOS scheduler is started, which then runs the ``main_task``, leading to the execution of ``app_main``.

This process is explained in detail in the following sections.

.. _first-stage-bootloader:

First stage (ROM) bootloader
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. only:: SOC_HP_CPU_HAS_MULTIPLE_CORES

    After SoC reset, PRO CPU will start running immediately, executing reset vector code, while APP CPU will be held in reset. During startup process, PRO CPU does all the initialization. APP CPU reset is de-asserted in the ``call_start_cpu0`` function of application startup code. Reset vector code is located in the mask ROM of the {IDF_TARGET_NAME} chip and cannot be modified.

.. only:: not SOC_HP_CPU_HAS_MULTIPLE_CORES

    After SoC reset, the CPU will start running immediately to perform initialization. The reset vector code is located in the mask ROM of the {IDF_TARGET_NAME} chip and cannot be modified.

Startup code called from the reset vector determines the boot mode by checking ``GPIO_STRAP_REG`` register for bootstrap pin states. Depending on the reset reason, the following takes place:

.. list::

    :ESP_ROM_SUPPORT_DEEP_SLEEP_WAKEUP_STUB: #. Reset from deep sleep: if the value in ``RTC_CNTL_STORE6_REG`` is non-zero, and CRC value of RTC memory in ``RTC_CNTL_STORE7_REG`` is valid, use ``RTC_CNTL_STORE6_REG`` as an entry point address and jump immediately to it. If ``RTC_CNTL_STORE6_REG`` is zero, or ``RTC_CNTL_STORE7_REG`` contains invalid CRC, or once the code called via ``RTC_CNTL_STORE6_REG`` returns, proceed with boot as if it was a power-on reset. **Note**: to run customized code at this point, a deep sleep stub mechanism is provided. Please see :doc:`deep sleep <deep-sleep-stub>` documentation for this.

    #. For power-on reset, software SoC reset, and watchdog SoC reset: check the ``GPIO_STRAP_REG`` register if a custom boot mode (such as UART Download Mode) is requested. If this is the case, this custom loader mode is executed from ROM. Otherwise, proceed with boot as if it was due to software CPU reset. Consult {IDF_TARGET_NAME} datasheet for a description of SoC boot modes and how to execute them.

    #. For software CPU reset and watchdog CPU reset: configure SPI flash based on EFUSE values, and attempt to load the code from flash. This step is described in more detail in the next paragraphs.

.. note::

    During normal boot modes the RTC watchdog is enabled when this happens, so if the process is interrupted or stalled then the watchdog will reset the SOC automatically and repeat the boot process. This may cause the SoC to strap into a new boot mode, if the strapping GPIOs have changed.

.. only:: esp32

    Second stage bootloader binary image is loaded from flash starting at address {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH}. If :doc:`/security/secure-boot-v1` is in use then the first 4 kB sector of flash is used to store secure boot IV and digest of the bootloader image. Otherwise, this sector is unused.

.. only:: esp32s2

    Second stage bootloader binary image is loaded from flash starting at address {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH}. The 4 kB sector of flash before this address is unused.

.. only:: SOC_KEY_MANAGER_SUPPORTED

    Second stage bootloader binary image is loaded from flash starting at address {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH}. The 8 kB sector of flash before this address is reserved for the key manager for use with flash encryption (AES-XTS).

.. only:: not (esp32 or esp32s2 or SOC_KEY_MANAGER_SUPPORTED)

    Second stage bootloader binary image is loaded from the start of flash at offset {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH}.

.. TODO: describe application binary image format, describe optional flash configuration commands.

.. _second-stage-bootloader:

Second Stage Bootloader
^^^^^^^^^^^^^^^^^^^^^^^

In ESP-IDF, the binary image which resides at offset {IDF_TARGET_CONFIG_BOOTLOADER_OFFSET_IN_FLASH} in flash is the second stage bootloader. Second stage bootloader source code is available in :idf:`components/bootloader` directory of ESP-IDF. Second stage bootloader is used in ESP-IDF to add flexibility to flash layout (using partition tables), and allow for various flows associated with flash encryption, secure boot, and over-the-air updates (OTA) to take place.

When the first stage (ROM) bootloader is finished checking and loading the second stage bootloader, it jumps to the second stage bootloader entry point found in the binary image header.

Second stage bootloader reads the partition table found by default at offset {IDF_TARGET_CONFIG_PARTITION_TABLE_OFFSET} (:ref:`configurable value <CONFIG_PARTITION_TABLE_OFFSET>`). See :doc:`partition tables <partition-tables>` documentation for more information. The bootloader finds factory and OTA app partitions. If OTA app partitions are found in the partition table, the bootloader consults the ``otadata`` partition to determine which one should be booted. See :doc:`/api-reference/system/ota` for more information.

For a full description of the configuration options available for the ESP-IDF bootloader, see :doc:`bootloader`.

For the selected partition, second stage bootloader reads the binary image from flash one segment at a time:

- For segments with load addresses in internal :ref:`iram` or :ref:`dram`, the contents are copied from flash to the load address.
- For segments which have load addresses in :ref:`drom` or :ref:`irom` regions, the flash MMU is configured to provide the correct mapping from the flash to the load address.

.. only:: esp32

    Note that the second stage bootloader configures flash MMU for both PRO and APP CPUs, but it only enables flash MMU for PRO CPU. Reason for this is that second stage bootloader code is loaded into the memory region used by APP CPU cache. The duty of enabling cache for APP CPU is passed on to the application.

Once all segments are processed - meaning code is loaded and flash MMU is set up, second stage bootloader verifies the integrity of the application and then jumps to the application entry point found in the binary image header.

.. _application-startup:

Application Startup
^^^^^^^^^^^^^^^^^^^

Application startup covers everything that happens after the app starts executing and before the ``app_main`` function starts running inside the main task. This is split into three stages:

- Port initialization of hardware and basic C runtime environment.
- System initialization of software services and FreeRTOS.
- Running the main task and calling ``app_main``.

.. note::

   Understanding all stages of ESP-IDF app initialization is often not necessary. To understand initialization from the application developer's perspective only, skip forward to :ref:`app-main-task`.

Port Initialization
-------------------

ESP-IDF application entry point is ``call_start_cpu0`` function found in :idf_file:`components/esp_system/port/cpu_start.c`. This function is executed by the second stage bootloader, and never returns.

This port-layer initialization function initializes the basic C Runtime Environment ("CRT") and performs initial configuration of the SoC's internal hardware:

.. list::

   - Reconfigure CPU exceptions for the app (allowing app interrupt handlers to run, and causing :doc:`fatal-errors` to be handled using the options configured for the app rather than the simpler error handler provided by ROM).
   - If the option :ref:`CONFIG_BOOTLOADER_WDT_ENABLE` is not set then the RTC watchdog timer is disabled.
   - Initialize internal memory (data & bss).
   - Finish configuring the MMU cache.
   :SOC_SPIRAM_SUPPORTED: - Enable PSRAM if configured.
   - Set the CPU clocks to the frequencies configured for the project.
   :SOC_MEMPROT_SUPPORTED: - Initialize memory protection if configured.
   :esp32: - Reconfigure the main SPI flash based on the app header settings (necessary for compatibility with bootloader versions before ESP-IDF V4.0, see :ref:`bootloader-compatibility`).
   :SOC_HP_CPU_HAS_MULTIPLE_CORES: - If the app is configured to run on multiple cores, start the other core and wait for it to initialize as well (inside the similar "port layer" initialization function ``call_start_cpu1``).

.. only:: SOC_HP_CPU_HAS_MULTIPLE_CORES

    Once ``call_start_cpu0`` completes running, it calls the "system layer" initialization function ``start_cpu0`` found in :idf_file:`components/esp_system/startup.c`. Other cores will also complete port-layer initialization and call ``start_other_cores`` found in the same file.

.. only:: not SOC_HP_CPU_HAS_MULTIPLE_CORES

    Once ``call_start_cpu0`` completes running, it calls the "system layer" initialization function ``start_cpu0`` found in :idf_file:`components/esp_system/startup.c`.

System Initialization
---------------------

The main system initialization function is ``start_cpu0``. By default, this function is weak-linked to the function ``start_cpu0_default``. This means that it is possible to override this function to add some additional initialization steps.

The primary system initialization stage includes:

.. list::

   - Log information about this application (project name, :ref:`app-version`, etc.) if default log level enables this.
   - Initialize the heap allocator (before this point all allocations must be static or on the stack).
   - Initialize esp_libc component syscalls and time functions.
   - Configure the brownout detector.
   - Setup libc stdin, stdout, and stderr according to the :ref:`serial console configuration <CONFIG_ESP_CONSOLE_UART>`.
   :esp32: - Perform any security-related checks, including burning efuses that should be burned for this configuration (including :ref:`disabling ROM download mode on ESP32 V3 <CONFIG_SECURE_UART_ROM_DL_MODE>`, :ref:`CONFIG_ESP32_DISABLE_BASIC_ROM_CONSOLE`).
   :not esp32: - Perform any security-related checks, including burning efuses that should be burned for this configuration (including :ref:`permanently limiting ROM download modes <CONFIG_SECURE_UART_ROM_DL_MODE>`).
   - Initialize SPI flash API support.
   - Call global C++ constructors and any C functions marked with ``__attribute__((constructor))``.

Secondary system initialization allows individual components to be initialized. If a component has an initialization function annotated with the ``ESP_SYSTEM_INIT_FN`` macro, it will be called as part of secondary initialization. Component initialization functions have priorities assigned to them to ensure the desired initialization order. The priorities are documented in :component_file:`esp_system/system_init_fn.txt` and ``ESP_SYSTEM_INIT_FN`` definition in source code are checked against this file.

.. _app-main-task:

Running the Main Task
---------------------

After all other components are initialized, the main task is created and the FreeRTOS scheduler starts running.

After doing some more initialization tasks (that require the scheduler to have started), the main task runs the application-provided function ``app_main`` in the firmware.

The main task that runs ``app_main`` has a fixed RTOS priority (one higher than the minimum) and a :ref:`configurable stack size <CONFIG_ESP_MAIN_TASK_STACK_SIZE>`.

.. only:: SOC_HP_CPU_HAS_MULTIPLE_CORES

   The main task core affinity is also configurable: :ref:`CONFIG_ESP_MAIN_TASK_AFFINITY`.

Unlike normal FreeRTOS tasks (or embedded C ``main`` functions), the ``app_main`` task is allowed to return. If this happens, The task is cleaned up and the system will continue running with other RTOS tasks scheduled normally. Therefore, it is possible to implement ``app_main`` as either a function that creates other application tasks and then returns, or as a main application task itself.

.. only:: SOC_HP_CPU_HAS_MULTIPLE_CORES

    Second Core Startup
    -------------------

    A similar but simpler startup process happens on the APP CPU:

    When running system initialization, the code on PRO CPU sets the entry point for APP CPU, de-asserts APP CPU reset, and waits for a global flag to be set by the code running on APP CPU, indicating that it has started. Once this is done, APP CPU jumps to ``call_start_cpu1`` function in :idf_file:`components/esp_system/port/cpu_start.c`.

    While PRO CPU does initialization in ``start_cpu0`` function, APP CPU runs ``start_cpu_other_cores`` function. Similar to ``start_cpu0``, this function is weak-linked and defaults to the ``start_cpu_other_cores_default`` function but can be replaced with a different function by the application.

    The ``start_cpu_other_cores_default`` function does some core-specific system initialization and then waits for the PRO CPU to start the FreeRTOS scheduler, at which point it executes ``esp_startup_start_app_other_cores`` which is another weak-linked function defaulting to ``esp_startup_start_app_other_cores_default``.

    By default ``esp_startup_start_app_other_cores_default`` does nothing but spin in a busy-waiting loop until the scheduler of the PRO CPU triggers an interrupt to start the RTOS scheduler on the APP CPU.
````

## File: docs/en/api-guides/stdio.rst
````
Standard I/O and Console Output
===============================

ESP-IDF provides C standard I/O facilities, such as ``stdin``, ``stdout``, and ``stderr`` streams, as well as C standard library functions such as ``printf()`` which operate on these streams.

As common in POSIX systems, these streams are buffering wrappers around file descriptors:

- ``stdin`` is a buffered stream for reading input from the user, wrapping file descriptor ``STDIN_FILENO`` (0).
- ``stdout`` is a buffered stream for writing output to the user, wrapping ``STDOUT_FILENO`` (1).
- ``stderr`` is a buffered stream for writing error messages to the user, wrapping ``STDERR_FILENO`` (2).

In ESP-IDF, there is no practical distinction between ``stdout`` and ``stderr``, as both streams are sent to the same physical interface. Most applications will use only ``stdout``. For example, ESP-IDF logging functions always write to ``stdout`` regardless of the log level.

The underlying stdin, stdout, and stderr file descriptors are implemented based on :doc:`VFS drivers <../api-reference/storage/vfs>`.

On {IDF_TARGET_NAME}, ESP-IDF provides implementations of VFS drivers for I/O over:

.. list::

    - UART
    :SOC_USB_SERIAL_JTAG_SUPPORTED: - USB Serial/JTAG
    :esp32s2 or esp32s3: - USB CDC (using USB_OTG peripheral)
    - "Null" (no output)

Standard I/O is not limited to these options, though. See below on enabling custom destinations for standard I/O.

Configuration
-------------

Built-in implementations of standard I/O can be selected using several Kconfig options:

.. list::

    - :ref:`CONFIG_ESP_CONSOLE_UART_DEFAULT<CONFIG_ESP_CONSOLE_UART_DEFAULT>` — Enables UART with default options (pin numbers, baud rate) for standard I/O.
    - :ref:`CONFIG_ESP_CONSOLE_UART_CUSTOM<CONFIG_ESP_CONSOLE_UART_CUSTOM>` — Enables UART for standard I/O, with TX/RX pin numbers and baud rate configurable via Kconfig.
    :esp32s2 or esp32s3: - :ref:`CONFIG_ESP_CONSOLE_USB_CDC<CONFIG_ESP_CONSOLE_USB_CDC>` — Enables USB CDC (using USB_OTG peripheral) for standard I/O. See :doc:`usb-otg-console` for details about hardware connections required.
    :SOC_USB_SERIAL_JTAG_SUPPORTED: - :ref:`CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG<CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG>` — Enables USB Serial/JTAG for standard I/O. See :doc:`usb-serial-jtag-console` for details about hardware connections required.
    - :ref:`CONFIG_ESP_CONSOLE_NONE<CONFIG_ESP_CONSOLE_NONE>` — Disables standard I/O. If this option is selected, ``stdin``, ``stdout``, and ``stderr`` will be mapped to ``/dev/null`` and won't produce any output or generate any input.

Enabling one of these option will cause the corresponding VFS driver to be built into the application and used to open ``stdin``, ``stdout``, and ``stderr`` streams. Data written to ``stdout`` and ``stderr`` will be sent over the selected interface, and input from the selected interface will be available on ``stdin``.

.. only:: SOC_USB_SERIAL_JTAG_SUPPORTED

    Secondary output
    ^^^^^^^^^^^^^^^^

    ESP-IDF has built-in support for sending standard output to a secondary destination. This option makes the application output visible on two interfaces at once, for example on both UART and USB Serial/JTAG.

    Note that secondary console is output-only:

        - data written to ``stdout`` and ``stderr`` by the application will be sent to both primary and secondary consoles
        - ``stdin`` will only contain data sent by the host to the primary console.

    The following secondary console options are available:

        - :ref:`CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG<CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG>`

Standard Streams and FreeRTOS Tasks
-----------------------------------

In ESP-IDF, to save RAM, ``FILE`` objects for ``stdin``, ``stdout``, and ``stderr`` are shared between all FreeRTOS tasks, but the pointers to these objects are unique for every task. This means that:

- It is possible to change ``stdin``, ``stdout``, and ``stderr`` for any given task without affecting other tasks, e.g., by doing ``stdin = fopen("/dev/uart/1", "r")``.
- To change the default ``stdin``, ``stdout``, ``stderr`` streams for new tasks, modify ``_GLOBAL_REENT->_stdin`` (``_stdout``, ``_stderr``) before creating the task.
- Closing default ``stdin``, ``stdout``, or ``stderr`` using ``fclose`` closes the ``FILE`` stream object, which will affect all other tasks.

Each stream (``stdin``, ``stdout``, ``stderr``) has a mutex associated with it. This mutex is used to protect the stream from concurrent access by multiple tasks. For example, if two tasks are writing to ``stdout`` at the same time, the mutex will ensure that the outputs from each task are not mixed together.

Blocking and non-blocking I/O
-----------------------------

UART
^^^^

By default, UART VFS uses simplified functions for reading from and writing to UART. Writes busy-wait until all data is put into UART FIFO, and reads are non-blocking, returning only the data present in the FIFO. Due to this non-blocking read behavior, higher level C library calls, such as ``fscanf("%d\n", &var);``, might not have desired results.

Applications which use the UART driver can instruct VFS to use the driver's interrupt driven, blocking read and write functions instead. This can be done using a call to the :cpp:func:`uart_vfs_dev_use_driver` function. It is also possible to revert to the basic non-blocking functions using a call to :cpp:func:`uart_vfs_dev_use_nonblocking`.

When the interrupt-driven driver is installed, it is also possible to enable/disable non-blocking behavior using ``fcntl`` function with ``O_NONBLOCK`` flag.

.. only:: SOC_USB_SERIAL_JTAG_SUPPORTED

    USB Serial/JTAG
    ^^^^^^^^^^^^^^^

    Similar to UART, the VFS driver for USB Serial/JTAG defaults to a simplified implementation: writes are blocking (busy-wait until all the data has been sent) and reads are non-blocking, returning only the data present in the FIFO. This behavior can be changed to use the interrupt driven, blocking read and write functions of USB Serial/JTAG driver using a call to the :cpp:func:`usb_serial_jtag_vfs_use_nonblocking` function. Note that the USB Serial/JTAG driver has to be initialized using :cpp:func:`usb_serial_jtag_driver_install` beforehand. It is also possible to revert to the basic non-blocking functions using a call to :cpp:func:`usb_serial_jtag_vfs_use_nonblocking`.

    When the interrupt-driven driver is installed, it is also possible to enable/disable non-blocking behavior using ``fcntl`` function with ``O_NONBLOCK`` flag.

.. only:: esp32s2 or esp32s3

    USB CDC (using USB_OTG peripheral)
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    USB CDC VFS driver provides blocking I/O behavior by default. It is possible to enable non-blocking behavior using ``fcntl`` function with ``O_NONBLOCK`` flag.

Newline conversion
------------------

VFS drivers provide an optional newline conversion feature for input and output. Internally, most applications send and receive lines terminated by the LF (``\n``) character. Different terminal programs may require different line termination, such as CR or CRLF.

Applications can configure this behavior globally using the following Kconfig options:

    - :ref:`CONFIG_LIBC_STDOUT_LINE_ENDING_CRLF<CONFIG_LIBC_STDOUT_LINE_ENDING_CRLF>`, :ref:`CONFIG_LIBC_STDOUT_LINE_ENDING_CR<CONFIG_LIBC_STDOUT_LINE_ENDING_CR>`, :ref:`CONFIG_LIBC_STDOUT_LINE_ENDING_LF<CONFIG_LIBC_STDOUT_LINE_ENDING_LF>` - for output
    - :ref:`CONFIG_LIBC_STDIN_LINE_ENDING_CRLF<CONFIG_LIBC_STDIN_LINE_ENDING_CRLF>`, :ref:`CONFIG_LIBC_STDIN_LINE_ENDING_CR<CONFIG_LIBC_STDIN_LINE_ENDING_CR>`, :ref:`CONFIG_LIBC_STDIN_LINE_ENDING_LF<CONFIG_LIBC_STDIN_LINE_ENDING_LF>` - for input


It is also possible to configure line ending conversion for the specific VFS driver:

.. list::

    - For UART: :cpp:func:`uart_vfs_dev_port_set_rx_line_endings` and :cpp:func:`uart_vfs_dev_port_set_tx_line_endings`
    :SOC_USB_SERIAL_JTAG_SUPPORTED: - For USB Serial/JTAG: :cpp:func:`usb_serial_jtag_vfs_set_rx_line_endings` and :cpp:func:`usb_serial_jtag_vfs_set_tx_line_endings`
    :esp32s2 or esp32s3: - For USB CDC (using USB_OTG peripheral): :cpp:func:`esp_vfs_dev_cdcacm_set_rx_line_endings` and :cpp:func:`esp_vfs_dev_cdcacm_set_tx_line_endings`

Buffering
---------

By default, standard I/O streams are line buffered. This means that data written to the stream is not sent to the underlying device until a newline character is written, or the buffer is full. This means, for example, that if you call ``printf("Hello")``, the text will not be sent to the UART until you call ``printf("\n")`` or the stream buffer fills up due to other prints.

This behavior can be changed using the ``setvbuf()`` function. For example, to disable buffering for ``stdout``:

.. code-block:: c

    setvbuf(stdout, NULL, _IONBF, 0);

You can also use ``setvbuf()`` to increase the buffer size, or switch to fully buffered mode.

Custom channels for standard I/O
--------------------------------

To send application output to a custom channel (for example, a WebSocket connection), it is possible to create a custom VFS driver. See the :doc:`VFS documentation <../api-reference/storage/vfs>` for details. The VFS driver has to implement at least the following functions:

    - ``open()`` and ``close()``
    - ``write()``
    - ``read()`` — only if the custom channel is also used for input
    - ``fstat()`` — recommended, to provide correct buffering behavior for the I/O streams
    - ``fcntl()`` — only if non-blocking I/O has to be supported

Once you have created a custom VFS driver, use :cpp:func:`esp_vfs_register_fs()` to register it with VFS. Then, use ``fopen()`` to redirect ``stdout`` and ``stderr`` to the custom channel. For example:

.. code-block:: c

    FILE *f = fopen("/dev/mychannel", "w");
    if (f == NULL) {
        // handle the error here
    }
    stdout = f;
    stderr = f;

Note that logging functions (``ESP_LOGE()``, etc.) write their output to ``stdout``. Keep this in mind when using logging within the implementation of your custom VFS (or any components which it calls). For example, if the custom VFS driver's ``write()`` operation fails and uses ``ESP_LOGE()`` to log the error, this will cause the output to be sent to ``stdout``, which would again call the custom VFS driver's ``write()`` operation. This would result in an infinite loop. It is recommended to keep track of this re-entry condition in the VFS driver's ``write()`` implementation, and return immediately if the write operation is still in progress.
````

## File: docs/en/api-guides/thread-local-storage.rst
````
Thread Local Storage
====================

:link_to_translation:`zh_CN:[中文]`

Overview
--------

Thread-local storage (TLS) is a mechanism by which variables are allocated such that there is one instance of the variable per extant thread. ESP-IDF provides three ways to make use of such variables:

- :ref:`freertos-native`: ESP-IDF FreeRTOS native APIs.
- :ref:`pthread-api`: ESP-IDF pthread APIs.
- :ref:`c11-std`: C11 standard introduces special keywords to declare variables as thread local.

.. _freertos-native:

FreeRTOS Native APIs
--------------------

The ESP-IDF FreeRTOS provides the following APIs to manage thread local variables:

- :cpp:func:`vTaskSetThreadLocalStoragePointer`
- :cpp:func:`pvTaskGetThreadLocalStoragePointer`
- :cpp:func:`vTaskSetThreadLocalStoragePointerAndDelCallback`

In this case, the maximum number of variables that can be allocated is limited by :ref:`CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS`. Variables are kept in the task control block (TCB) and accessed by their index. Note that index 0 is reserved for ESP-IDF internal uses.

Using the APIs above, you can allocate thread local variables of an arbitrary size, and assign them to any number of tasks. Different tasks can have different sets of TLS variables.

If size of the variable is more than 4 bytes, then you need to allocate/deallocate memory for it. Variable's deallocation is initiated by FreeRTOS when task is deleted, but user must provide callback function to do proper cleanup.

.. _pthread-api:

Pthread APIs
----------------

The ESP-IDF provides the following :doc:`/api-reference/system/pthread` to manage thread local variables:

- :cpp:func:`pthread_key_create`
- :cpp:func:`pthread_key_delete`
- :cpp:func:`pthread_getspecific`
- :cpp:func:`pthread_setspecific`

These APIs have all benefits of the ones above, but eliminates some their limits. The number of variables is limited only by size of available memory on the heap. Due to the dynamic nature, this API introduces additional performance overhead compared to the native one.

.. _c11-std:

C11 Standard
------------

The ESP-IDF FreeRTOS supports thread local variables according to C11 standard, ones specified with ``__thread`` keyword. For details on this feature, please refer to the `GCC documentation <https://gcc.gnu.org/onlinedocs/gcc-5.5.0/gcc/Thread-Local.html#Thread-Local>`_.

Storage for that kind of variables is allocated on the task stack. Note that area for all such variables in the program is allocated on the stack of every task in the system even if that task does not use such variables at all. For example, ESP-IDF system tasks (e.g., ``ipc``, ``timer`` tasks etc.) will also have that extra stack space allocated. Thus feature should be used with care.

Using C11 thread local variables comes at a trade-off. On one hand, they are quite handy to use in programming and can be accessed using minimal CPU instructions. However, this benefit comes at the cost of additional stack usage for all tasks in the system. Due to static nature of variables allocation, all tasks in the system have the same sets of C11 thread local variables.
````

## File: docs/en/api-guides/unit-tests.rst
````
Unit Testing in {IDF_TARGET_NAME}
=================================

:link_to_translation:`zh_CN:[中文]`

ESP-IDF provides the following methods to test software.

- Target based tests using a central unit test application which runs on the {IDF_TARGET_PATH_NAME}. These tests use the `Unity <https://www.throwtheswitch.org/unity>`_ unit test framework. They can be integrated into an ESP-IDF component by placing them in the component's ``test`` subdirectory. This document mainly introduces this target based tests.

- Linux-host based unit tests in which part of the hardware can be abstracted via mocks. Currently, Linux-host based tests are still under development and only a small fraction of IDF components support them. More information on running IDF applications on the host can be found here: :doc:`Running Applications on the Host Machine <host-apps>`.

Normal Test Cases
-----------------

Unit tests are located in the ``test`` subdirectory of a component. Tests are written in C, and a single C source file can contain multiple test cases. Test files start with the word "test".

Each test file should include the ``unity.h`` header and the header for the C module to be tested.

Tests are added in a function in the C file as follows:

.. code-block:: c

    TEST_CASE("test name", "[module name]")
    {
            // Add test here
    }

- The first argument is a descriptive name for the test.
- The second argument is an identifier in square brackets. Identifiers are used to group related test, or tests with specific properties.

.. note::
    There is no need to add a main function with ``UNITY_BEGIN()`` and ``​UNITY_END()`` in each test case. ``unity_platform.c`` will run ``UNITY_BEGIN()`` autonomously, and run the test cases, then call ``​UNITY_END()``.

The ``test`` subdirectory should contain a :ref:`component CMakeLists.txt <component-directories>`, since they are themselves components (i.e., a test component). ESP-IDF uses the Unity test framework located in the ``unity`` component. Thus, each test component should specify the ``unity`` component as a component requirement using the ``REQUIRES`` argument. Normally, components :ref:`should list their sources manually <cmake-file-globbing>`; for component tests however, this requirement is relaxed and the use of the ``SRC_DIRS`` argument in ``idf_component_register`` is advised.

Overall, the minimal ``test`` subdirectory ``CMakeLists.txt`` file should contain the following:

.. code:: cmake

    idf_component_register(SRC_DIRS "."
                           INCLUDE_DIRS "."
                           REQUIRES unity)

See http://www.throwtheswitch.org/unity for more information about writing tests in Unity.


Multi-device Test Cases
-----------------------

The normal test cases will be executed on one DUT (Device Under Test). However, components that require some form of communication (e.g., GPIO, SPI) require another device to communicate with, thus cannot be tested through normal test cases. Multi-device test cases involve writing multiple test functions, and running them on multiple DUTs.

The following is an example of a multi-device test case:

.. code-block:: c

    void gpio_master_test()
    {
        gpio_config_t slave_config = {
                .pin_bit_mask = 1 << MASTER_GPIO_PIN,
                .mode = GPIO_MODE_INPUT,
        };
        gpio_config(&slave_config);
        unity_wait_for_signal("output high level");
        TEST_ASSERT(gpio_get_level(MASTER_GPIO_PIN) == 1);
    }

    void gpio_slave_test()
    {
        gpio_config_t master_config = {
                .pin_bit_mask = 1 << SLAVE_GPIO_PIN,
                .mode = GPIO_MODE_OUTPUT,
        };
        gpio_config(&master_config);
        gpio_set_level(SLAVE_GPIO_PIN, 1);
        unity_send_signal("output high level");
    }

    TEST_CASE_MULTIPLE_DEVICES("gpio multiple devices test example", "[driver]", gpio_master_test, gpio_slave_test);

The macro ``TEST_CASE_MULTIPLE_DEVICES`` is used to declare a multi-device test case.

- The first argument is test case name.
- The second argument is test case description.
- From the third argument, up to 5 test functions can be defined, each function will be the entry point of tests running on each DUT.

Running test cases from different DUTs could require synchronizing between DUTs. We provide ``unity_wait_for_signal`` and ``unity_send_signal`` to support synchronizing with UART. As the scenario in the above example, the slave should get GPIO level after master set level. DUT UART console will prompt and user interaction is required:

DUT1 (master) console::

    Waiting for signal: [output high level]!
    Please press "Enter" key to once any board send this signal.

DUT2 (slave) console::

    Send signal: [output high level]!

Once the signal is sent from DUT2, you need to press "Enter" on DUT1, then DUT1 unblocks from ``unity_wait_for_signal`` and starts to change GPIO level.


Multi-stage Test Cases
----------------------

The normal test cases are expected to finish without reset (or only need to check if reset happens). Sometimes we expect to run some specific tests after certain kinds of reset. For example, we want to test if the reset reason is correct after a wake up from deep sleep. We need to create a deep-sleep reset first and then check the reset reason. To support this, we can define multi-stage test cases, to group a set of test functions::

    static void trigger_deepsleep(void)
    {
        esp_sleep_enable_timer_wakeup(2000);
        esp_deep_sleep_start();
    }

    void check_deepsleep_reset_reason()
    {
        soc_reset_reason_t reason = esp_rom_get_reset_reason(0);
        TEST_ASSERT(reason == RESET_REASON_CORE_DEEP_SLEEP);
    }

    TEST_CASE_MULTIPLE_STAGES("reset reason check for deepsleep", "[{IDF_TARGET_PATH_NAME}]", trigger_deepsleep, check_deepsleep_reset_reason);

Multi-stage test cases present a group of test functions to users. It needs user interactions (select cases and select different stages) to run the case.

Tests For Different Targets
---------------------------

Some tests (especially those related to hardware) cannot run on all targets. Below is a guide how to make your unit tests run on only specified targets.

1. Wrap your test code by ``!(TEMPORARY_)DISABLED_FOR_TARGETS()`` macros and place them either in the original test file, or separate the code into files grouped by functions, but make sure all these files will be processed by the compiler. E.g.::

      #if !TEMPORARY_DISABLED_FOR_TARGETS(ESP32, ESP8266)
      TEST_CASE("a test that is not ready for esp32 and esp8266 yet", "[]")
      {
      }
      #endif //!TEMPORARY_DISABLED_FOR_TARGETS(ESP32, ESP8266)

Once you need one of the tests to be compiled on a specified target, just modify the targets in the disabled list. It's more encouraged to use some general conception that can be described in ``soc_caps.h`` to control the disabling of tests. If this is done but some of the tests are not ready yet, use both of them (and remove ``!(TEMPORARY_)DISABLED_FOR_TARGETS()`` later). E.g.: ::

      #if SOC_SDIO_SLAVE_SUPPORTED
      #if !TEMPORARY_DISABLED_FOR_TARGETS(ESP64)
      TEST_CASE("a sdio slave tests that is not ready for esp64 yet", "[sdio_slave]")
      {
          //available for esp32 now, and will be available for esp64 in the future
      }
      #endif //!TEMPORARY_DISABLED_FOR_TARGETS(ESP64)
      #endif //SOC_SDIO_SLAVE_SUPPORTED

2. For test code that you are 100% for sure that will not be supported (e.g., no peripheral at all), use ``DISABLED_FOR_TARGETS``; for test code that should be disabled temporarily, or due to lack of runners, etc., use ``TEMPORARY_DISABLED_FOR_TARGETS``.

Some old ways of disabling unit tests for targets, that have obvious disadvantages, are deprecated:

- DON'T put the test code under ``test/target`` folder and use CMakeLists.txt to choose one of the target folder. This is prevented because test code is more likely to be reused than the implementations. If you put something into ``test/esp32`` just to avoid building it on esp32s2, it's hard to make the code tidy if you want to enable the test again on esp32s3.

- DON'T use ``CONFIG_IDF_TARGET_xxx`` macros to disable the test items any more. This makes it harder to track disabled tests and enable them again. Also, a black-list style ``#if !disabled`` is preferred to white-list style ``#if CONFIG_IDF_TARGET_xxx``, since you will not silently disable cases when new targets are added in the future. But for test implementations, it's allowed to use ``#if CONFIG_IDF_TARGET_xxx`` to pick one of the implementation code.

  - Test item: some items that will be performed on some targets, but skipped on other targets. E.g.

    There are three test items SD 1-bit, SD 4-bit and SDSPI. For ESP32-S2, which doesn't have SD host, among the tests only SDSPI is enabled on ESP32-S2.

  - Test implementation: some code will always happen, but in different ways. E.g.

    There is no SDIO PKT_LEN register on ESP8266. If you want to get the length from the slave as a step in the test process, you can have different implementation code protected by ``#if CONFIG_IDF_TARGET_`` reading in different ways.

    But please avoid using ``#else`` macro. When new target is added, the test case will fail at building stage, so that the maintainer will be aware of this, and choose one of the implementations explicitly.

Building Unit Test Apps
-----------------------

Follow the setup instructions in the top-level esp-idf README. Make sure that ``IDF_PATH`` environment variable is set to point to the path of esp-idf top-level directory.

Change into the test app directory to configure and build it:

* ``idf.py menuconfig`` - configure unit test app.
* ``idf.py build`` - build unit test app.


When the build finishes, it will print instructions for flashing the chip. You can simply run ``idf.py flash`` to flash all build output.

Running Unit Tests
------------------

.. note::

    We also provide the pytest-based framework `pytest-embedded <https://github.com/espressif/pytest-embedded>`_ to help make running unit-tests more convenient and efficient. If you need to run tests in CI or run multiple tests in a row we recommend checking out this project. For more information see `Pytest-embedded Docs <https://docs.espressif.com/projects/pytest-embedded/en/latest/>`_ and :doc:`/contribute/esp-idf-tests-with-pytest`.


After flashing reset the {IDF_TARGET_NAME} and it will boot the unit test app.

When unit test app is idle, press "Enter" will make it print test menu with all available tests::

    Here's the test menu, pick your combo:
    (1)     "esp_ota_begin() verifies arguments" [ota]
    (2)     "esp_ota_get_next_update_partition logic" [ota]
    (3)     "Verify bootloader image in flash" [bootloader_support]
    (4)     "Verify unit test app image" [bootloader_support]
    (5)     "can use new and delete" [cxx]
    (6)     "can call virtual functions" [cxx]
    (7)     "can use static initializers for non-POD types" [cxx]
    (8)     "can use std::vector" [cxx]
    (9)     "static initialization guards work as expected" [cxx]
    (10)    "global initializers run in the correct order" [cxx]
    (11)    "before scheduler has started, static initializers work correctly" [cxx]
    (12)    "adc2 work with wifi" [adc]
    (13)    "gpio master/slave test example" [ignore][misc][test_env=UT_T2_1][multi_device]
            (1)     "gpio_master_test"
            (2)     "gpio_slave_test"
    (14)    "SPI Master clockdiv calculation routines" [spi]
    (15)    "SPI Master test" [spi][ignore]
    (16)    "SPI Master test, interaction of multiple devs" [spi][ignore]
    (17)    "SPI Master no response when switch from host1 (SPI2) to host2 (SPI3)" [spi]
    (18)    "SPI Master DMA test, TX and RX in different regions" [spi]
    (19)    "SPI Master DMA test: length, start, not aligned" [spi]
    (20)    "reset reason check for deepsleep" [{IDF_TARGET_PATH_NAME}][test_env=UT_T2_1][multi_stage]
            (1)     "trigger_deepsleep"
            (2)     "check_deepsleep_reset_reason"

The normal case will print the case name and description. Master-slave cases will also print the sub-menu (the registered test function names).

Test cases can be run by inputting one of the following:

- Test case name in quotation marks to run a single test case
- Test case index to run a single test case
- Module name in square brackets to run all test cases for a specific module
- An asterisk to run all test cases

``[multi_device]`` and ``[multi_stage]`` tags tell the test runner whether a test case is a multiple devices or multiple stages of test case. These tags are automatically added by ```TEST_CASE_MULTIPLE_STAGES`` and ``TEST_CASE_MULTIPLE_DEVICES`` macros.

After you select a multi-device test case, it will print sub-menu::

    Running gpio master/slave test example...
    gpio master/slave test example
            (1)     "gpio_master_test"
            (2)     "gpio_slave_test"

You need to input a number to select the test running on the DUT.

Similar to multi-device test cases, multi-stage test cases will also print sub-menu::

    Running reset reason check for deepsleep...
    reset reason check for deepsleep
            (1)     "trigger_deepsleep"
            (2)     "check_deepsleep_reset_reason"

First time you execute this case, input ``1`` to run first stage (trigger deepsleep). After DUT is rebooted and able to run test cases, select this case again and input ``2`` to run the second stage. The case only passes if the last stage passes and all previous stages trigger reset.

Project Structure and Automated Workflows
-----------------------------------------

A good starting point on how to structure and test an application is the `Github ESP Test Template <https://github.com/espressif/gh-esp-test-template>`_ project. It shows how to set up and run tests both in simulation and on real hardware, using GitHub CI workflows.

For more complex projects, tests would typically be split into several test-apps, testing each component separately. In these cases, you can use the tool `IDF Build Apps <https://github.com/espressif/idf-build-apps>`_ to automate finding and building all test-apps in your project.


.. _cache-compensated-timer:

Timing Code with Cache Compensated Timer
----------------------------------------

Instructions and data stored in external memory (e.g., SPI Flash and SPI RAM) are accessed through the CPU's unified instruction and data cache. When code or data is in cache, access is very fast (i.e., a cache hit).

However, if the instruction or data is not in cache, it needs to be fetched from external memory (i.e., a cache miss). Access to external memory is significantly slower, as the CPU must execute stall cycles whilst waiting for the instruction or data to be retrieved from external memory. This can cause the overall code execution speed to vary depending on the number of cache hits or misses.

Code and data placements can vary between builds, and some arrangements may be more favorable with regards to cache access (i.e., minimizing cache misses). This can technically affect execution speed, however these factors are usually irrelevant as their effect 'average out' over the device's operation.

The effect of the cache on execution speed, however, can be relevant in benchmarking scenarios (especially micro benchmarks). There might be some variability in measured time between runs and between different builds. A technique for eliminating for some of the variability is to place code and data in instruction or data RAM (IRAM/DRAM), respectively. The CPU can access IRAM and DRAM directly, eliminating the cache out of the equation. However, this might not always be viable as the size of IRAM and DRAM is limited.

The cache compensated timer is an alternative to placing the code/data to be benchmarked in IRAM/DRAM. This timer uses the processor's internal event counters in order to determine the amount of time spent on waiting for code/data in case of a cache miss, then subtract that from the recorded wall time.

  .. code-block:: c

    // Start the timer
    ccomp_timer_start();

    // Function to time
    func_code_to_time();

    // Stop the timer, and return the elapsed time in microseconds relative to
    // ccomp_timer_start
    int64_t t = ccomp_timer_stop();


One limitation of the cache compensated timer is that the task that benchmarked functions should be pinned to a core. This is due to each core having its own event counters that are independent of each other. For example, if ``ccomp_timer_start`` gets called on one core, put to sleep by the scheduler, wakes up, and gets rescheduled on the other core, then the corresponding ``ccomp_timer_stop`` will be invalid.

.. _mocks:

Mocks
-----

.. note::
    Currently, mocking is only possible with some selected components when running on the Linux host. In the future, we plan to make essential components in IDF mock-able. This will also include mocking when running on the {IDF_TARGET_NAME}.

One of the biggest problems regarding unit testing on embedded systems are the strong hardware dependencies. Running unit tests directly on the {IDF_TARGET_NAME} can be especially difficult for higher layer components for the following reasons:

- Decreased test reliability due to lower layer components and/or hardware setup.
- Increased difficulty in testing edge cases due to limitations of lower layer components and/or hardware setup
- Increased difficulty in identifying the root cause due to the large number of dependencies influencing the behavior

When testing a particular component, (i.e., the component under test), mocking allows the dependencies of the component under test to be substituted (i.e., mocked) entirely in software. Through mocking, hardware details are emulated and specified at run time, but only if necessary. To allow mocking, ESP-IDF integrates the `CMock <https://www.throwtheswitch.org/cmock>`_ mocking framework as a component. With the addition of some CMake functions in the ESP-IDF build system, it is possible to conveniently mock the entirety (or a part) of an IDF component.

Ideally, all components that the component under test is dependent on should be mocked, thus allowing the test environment complete control over all interactions with the component under test. However, if mocking all dependent components becomes too complex or too tedious (e.g., because you need to mock too many function calls) you have the following options:

.. list::
    - Include more "real" IDF code in the tests. This may work but increases the dependency on the "real" code's behavior. Furthermore, once a test fails, you may not know if the failure is in your actual code under test or the "real" IDF code.
    - Re-evaluate the design of the code under test and attempt to reduce its dependencies by dividing the code under test into more manageable components. This may seem burdensome but it is quite common that unit tests expose software design weaknesses. Fixing design weaknesses will not only help with unit testing in the short term, but will help future code maintenance as well.

Refer to :component_file:`cmock/CMock/docs/CMock_Summary.md` for more details on how CMock works and how to create and use mocks.

Requirements
^^^^^^^^^^^^

Mocking with CMock requires ``Ruby`` on the host machine. Furthermore, since mocking currently only works on the Linux target, the requirements of the latter also need to be fulfilled:

.. include:: inc/linux-host-requirements.rst

Mock a Component
^^^^^^^^^^^^^^^^

If a mocked component, called a *component mock*, is already available in ESP-IDF, then it can be used right away as long as it satisfies the required functionality. Refer to :ref:`component-linux-mock-support` to see which components are mocked already. Then refer to :ref:`adjustments_for_mocks` in order to use the component mock.

It is necessary to create component mocks if they are not yet provided in ESP-IDF. To create a component mock, the component needs to be overwritten in a particular way. Overriding a component entails creating a component with the exact same name as the original component, then letting the build system discover it later than the original component (see :ref:`Multiple components with the same name <cmake-components-same-name>` for more details).

In the component mock, the following parts are specified:

.. list::
    - The headers providing the functions to generate mocks for
    - Include paths of the aforementioned headers
    - Dependencies of the mock component (this is necessary e.g. if the headers include files from other components)

All these parts have to be specified using the IDF build system function ``idf_component_mock``. You can use the IDF build system function ``idf_component_get_property`` with the tag ``COMPONENT_OVERRIDEN_DIR`` to access the component directory of the original component and then register the mock component parts using ``idf_component_mock``:

.. code:: none

    idf_component_get_property(original_component_dir <original-component-name> COMPONENT_OVERRIDEN_DIR)
    ...
    idf_component_mock(INCLUDE_DIRS "${original_component_dir}/include"
        REQUIRES freertos
        MOCK_HEADER_FILES ${original_component_dir}/include/header_containing_functions_to_mock.h)

The component mock also requires a separate ``mock`` directory containing a ``mock_config.yaml`` file that configures CMock. A simple ``mock_config.yaml`` could look like this:

  .. code-block:: yaml

    :cmock:
      :plugins:
        - expect
        - expect_any_args

For more details about the CMock configuration yaml file, have a look at :component_file:`cmock/CMock/docs/CMock_Summary.md`.

Note that the component mock does not have to mock the original component in its entirety. As long as the test project's dependencies and dependencies of other code to the original components are satisfied by the component mock, partial mocking is adequate. In fact, most of the component mocks in IDF in ``tools/mocks`` are only partially mocking the original component.

Examples of component mocks can be found under :idf:`tools/mocks` in the IDF directory. General information on how to *override an IDF component* can be found in :ref:`Multiple components with the same name <cmake-components-same-name>`. There are several examples for testing code while mocking dependencies with CMock (non-exhaustive list):

- :component_file:`unit test for the NVS Page class <nvs_flash/host_test/nvs_page_test/README.md>`.
- :component_file:`unit test for esp_event <esp_event/host_test/esp_event_unit_test/main/esp_event_test.cpp>`.
- :component_file:`unit test for mqtt <mqtt/esp-mqtt/host_test/README.md>`.

.. _adjustments_for_mocks:

Adjustments in Unit Test
^^^^^^^^^^^^^^^^^^^^^^^^

The unit test needs to inform the cmake build system to mock dependent components (i.e., it needs to override the original component with the mock component). This is done by either placing the component mock into the project's ``components`` directory or adding the mock component's directory using the following line in the project's root ``CMakeLists.txt``:

  .. code:: cmake

    list(APPEND EXTRA_COMPONENT_DIRS "<mock_component_dir>")

Both methods will override existing components in ESP-IDF with the component mock. The latter is particularly convenient if you use component mocks that are already supplied by IDF.

Users can refer to the ``esp_event`` host-based unit test and its :component_file:`esp_event/host_test/esp_event_unit_test/CMakeLists.txt` as an example of a component mock.

Application Examples
--------------------

:example:`system/unit_test` demonstrates how to use the Unity library to add unit tests to custom components in an {IDF_TARGET_NAME} development environment, showcasing features such as assertions, test registration, and the use of UNITY_BEGIN() and UNITY_END() macros.
````

## File: docs/en/api-guides/usb-otg-console.rst
````
USB OTG Console
***************

:link_to_translation:`zh_CN:[中文]`

On chips with an integrated USB peripheral, it is possible to use USB Communication Device Class (CDC) to implement the serial console, instead of using UART with an external USB-UART bridge chip. {IDF_TARGET_NAME} ROM code contains a USB CDC implementation, which supports for some basic functionality without requiring the application to include the USB stack:

* Bidirectional serial console, which can be used with :doc:`IDF Monitor <tools/idf-monitor>` or another serial monitor.
* Flashing using ``esptool.py`` and ``idf.py flash``.
* :doc:`Device Firmware Update (DFU) <dfu>` interface for flashing the device using ``dfu-util`` and ``idf.py dfu``.

.. note::

    At the moment, this "USB Console" feature is incompatible with TinyUSB stack. However, if TinyUSB is used, it can provide its own CDC implementation.

Hardware Requirements
=====================

Connect {IDF_TARGET_NAME} to the USB port as follows.

.. list-table::
    :header-rows: 1
    :widths: 50 50
    :align: center

    * - GPIO
      - USB
    * - 20
      - D+ (green)
    * - 19
      - D- (white)
    * - GND
      - GND (black)
    * -
      - +5V (red)

Some development boards may offer a USB connector for the internal USB peripheral — in that case, no extra connections are required.

.. only:: esp32s3

    By default, the :doc:`USB_SERIAL_JTAG <usb-serial-jtag-console>` module is connected to the internal PHY of ESP32-S3, while the USB OTG peripheral can be used only if the external USB PHY is connected. Since the CDC console is provided via the USB OTG peripheral, it cannot be used through the internal PHY in this configuration.

    You can permanently switch the internal USB PHY to work with USB OTG peripheral instead of USB_SERIAL_JTAG by burning ``USB_PHY_SEL`` eFuse. See ESP32-S3 Technical Reference Manual for more details about USB_SERIAL_JTAG and USB OTG.

    Note however that USB_SERIAL_JTAG also provides a CDC console, so enabling the CDC console should not be the primary reason for switching from USB_SERIAL_JTAG to USB CDC.


Software Configuration
======================

USB console feature can be enabled using ``CONFIG_ESP_CONSOLE_USB_CDC`` option in menuconfig tool (see :ref:`CONFIG_ESP_CONSOLE_UART`).

Once the option is enabled, build the project as usual.

Uploading the Application
=========================

.. _usb_console_initial_upload:

Initial Upload
--------------

If the {IDF_TARGET_NAME} is not yet flashed with a program that enables a USB console, an initial upload of the program is required. There are three alternative options to perform the initial upload.

Once the initial upload is done, the application will start up and a USB CDC port will appear in the system.

.. note::

    The port name may change after the initial upload, so check the port list again before running ``idf.py monitor``.


Initial Upload Using the ROM Download Mode, over USB CDC
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Place {IDF_TARGET_NAME} into download mode. To do this, keep GPIO0 low while toggling reset. On many development boards, the "Boot" button is connected to GPIO0, and you can press "Reset" button while holding "Boot".
* A serial port will appear in the system. On most operating systems (Windows 8 and later, Linux, macOS), driver installation is not required. Find the port name using Device Manager (Windows) or by listing ``/dev/ttyACM*`` devices on Linux or ``/dev/cu*`` devices on macOS.
* Run ``idf.py flash -p PORT`` to upload the application, with ``PORT`` determined in the previous step.

Initial Upload Using the ROM Download Mode, over USB DFU
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Place {IDF_TARGET_NAME} into download mode. To do this, keep GPIO0 low while toggling reset. On many development boards, the "Boot" button is connected to GPIO0, and you can press "Reset" button while holding "Boot".
* Run ``idf.py dfu-flash``.

See :ref:`api_guide_dfu_flash` for details about DFU flashing.

Initial Upload Using UART
^^^^^^^^^^^^^^^^^^^^^^^^^

On development boards with a USB-UART bridge, upload the application over UART: ``idf.py flash -p PORT`` where ``PORT`` is the name of the serial port provided by the USB-UART bridge.

Subsequent Usage
----------------

Once the application is uploaded for the first time, you can run ``idf.py flash`` and ``idf.py monitor`` as usual.

Limitations
===========

There are several limitations to the USB console feature. These may or may not be significant, depending on the type of application being developed, and the development workflow. Most of these limitations stem from the fact that USB CDC is implemented in software, so the console working over USB CDC is more fragile and complex than a console working over UART.

1. If the application crashes, panic handler output may not be sent over USB CDC in some cases. If the memory used by the CDC driver is corrupted, or there is some other system-level issue, CDC may not work for sending panic handler messages over USB. This does work in many situations, but is not guaranteed to work as reliably as the UART output does. Similarly, if the application enters a boot loop before the USB CDC driver has a chance to start up, there will be no console output.

2. If the application accidentally reconfigures the USB peripheral pins, or disables the USB peripheral, USB CDC device will disappear from the system. After fixing the issue in the application, you will need to follow the :ref:`usb_console_initial_upload` process to flash the application again.

3. If the application enters Light-sleep mode (including automatic Light-sleep) or Deep-sleep mode, USB CDC device will disappear from the system.

4. USB CDC driver reserves some amount of RAM and increases application code size. Keep this in mind if trying to optimize application memory usage.

5. By default, the low-level ``esp_rom_printf`` feature and ``ESP_EARLY_LOG`` are disabled when USB CDC is used. These can be enabled using :ref:`CONFIG_ESP_CONSOLE_USB_CDC_SUPPORT_ETS_PRINTF` option. With this option enabled, ``esp_rom_printf`` can be used, at the expense of increased IRAM usage. Keep in mind that the cost of ``esp_rom_printf`` and ``ESP_EARLY_LOG`` over USB CDC is significantly higher than over UART. This makes these logging mechanisms much less suitable for "printf debugging", especially in the interrupt handlers.

6. If you are developing an application which uses the USB peripheral with the TinyUSB stack, this USB Console feature can not be used. This is mainly due to the following reasons:

   * This feature relies on a different USB CDC software stack in {IDF_TARGET_NAME} ROM.
   * USB descriptors used by the ROM CDC stack may be different from the descriptors used by TinyUSB.
   * When developing applications which use USB peripheral, it is very likely that USB functionality will not work or will not fully work at some moments during development. This can be due to misconfigured USB descriptors, errors in the USB stack usage, or other reasons. In this case, using the UART console for flashing and monitoring provides a much better development experience.

7. When debugging the application using JTAG, USB CDC may stop working if the CPU is stopped on a breakpoint. USB CDC operation relies on interrupts from the USB peripheral being serviced periodically. If the host computer does not receive valid responses from the USB device side for some time, it may decide to disconnect the device. The actual time depends on the OS and the driver, and ranges from a few hundred milliseconds to a few seconds.
````

## File: docs/en/api-guides/usb-serial-jtag-console.rst
````
**********************************
USB Serial/JTAG Controller Console
**********************************

:link_to_translation:`zh_CN:[中文]`

Generally, ESP chips implement a serial port using UART and can be connected to a serial console emulator on a host/PC via an external USB-UART bridge chip. However, on ESP chips that contain a USB Serial/JTAG Controller, the CDC-ACM portion of the controller implements a serial port that is connected directly to a host/PC, thus does not require an external USB-UART bridge chip.

{IDF_TARGET_NAME} contains a USB Serial/JTAG Controller providing the following functions:

* Bidirectional serial console, which can be used with :doc:`IDF Monitor <tools/idf-monitor>` or another serial monitor.
* Flashing using ``esptool.py`` and ``idf.py flash``.
* JTAG debugging, performed simultaneously with serial operations using tools like OpenOCD.

.. note::

  The USB Serial/JTAG Controller is a fixed-function USB device that is implemented entirely in hardware, meaning that it cannot be reconfigured to perform any function other than a serial port and JTAG debugging functionality. This is in contrast to the USB OTG controllers in some ESP chips that can be configured to perform the function of multiple types of USB devices.

Hardware Requirements
=====================

{IDF_TARGET_USB_DP_GPIO:default="Not Updated!",esp32c3="19",esp32s3="20", esp32c6="13", esp32h2="27", esp32p4="25/27", esp32c5="14", esp32c61="13", esp32h4="14", esp32h21="18"}
{IDF_TARGET_USB_DM_GPIO:default="Not Updated!",esp32c3="18",esp32s3="19", esp32c6="12", esp32h2="26", esp32p4="24/26", esp32c5="13", esp32c61="12", esp32h4="13", esp32h21="17"}

Connect {IDF_TARGET_NAME} to the USB port as follows:

.. list-table::
    :header-rows: 1
    :widths: 40 60
    :align: center

    * - GPIO
      - USB
    * - {IDF_TARGET_USB_DP_GPIO}
      - D+ (green)
    * - {IDF_TARGET_USB_DM_GPIO}
      - D- (white)
    * - GND
      - GND (black)
    * - 5V (or externally supplied)
      - +5V (red)

Some development boards may offer a USB connector for the USB Serial/JTAG Controller. In that case, no extra connections are required.

Software Configuration
======================

The USB Serial/JTAG Controller can be used as the serial port by selecting ``CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG`` in the :ref:`CONFIG_ESP_CONSOLE_UART` option. Once selected, building and flashing the project can occur as usual.

Alternatively, you can access the output through the ``usb_serial_jtag`` port but make sure ``CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG`` is selected in the :ref:`CONFIG_ESP_CONSOLE_SECONDARY`.

.. warning::

    Besides output, if you also want to input or use REPL with the console, please select ``CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG``.

Uploading the Application
=========================

The USB Serial/JTAG Controller is able to put the {IDF_TARGET_NAME} into download mode automatically. Simply flash as usual, but specify the USB Serial/JTAG Controller port on your system: ``idf.py flash -p PORT``, where ``PORT`` is the name of the proper port.

.. note::

    The USB Serial/JTAG Controller's serial port usually appears:

    - as ``/dev/ttyACM*`` on Linux
    - as ``/dev/cu*`` on Mac
    - as a ``COM*`` port in the Windows Device Manager

Limitations
===========

{IDF_TARGET_BOOT_PIN:default = "Not Updated!", esp32c3 = "GPIO9", esp32s3 = "GPIO0", esp32c6 = "GPIO9"}

There are several limitations to the USB Serial/JTAG console feature. The significance of these limitations depends on the type of application being developed, and the development workflow.

{IDF_TARGET_BOOT_PIN:default = "Not Updated!", esp32c3 = "GPIO9", esp32s3 = "GPIO0", esp32c6 = "GPIO9"}

USB Pin Reconfiguration
-----------------------

If the application accidentally reconfigures the USB peripheral pins or disables the USB Serial/JTAG Controller, the device disappears from the system. After fixing the issue in the application, you need to manually put the {IDF_TARGET_NAME} into download mode by pulling low {IDF_TARGET_BOOT_PIN} and resetting the chip.

If the application enters Deep-sleep mode, the USB Serial/JTAG device disappears from the system.

Data Buffering
--------------

For data transmitted from {IDF_TARGET_NAME} to PC Terminal (e.g., stdout, logs), the {IDF_TARGET_NAME} first writes to a small internal buffer. After this buffer becomes full (for example, if no PC Terminal is connected), the {IDF_TARGET_NAME} does a one-time wait of 50 ms for the PC Terminal to request the data. This can appear as a very brief pause in your application.

For data transmitted from the PC Terminal to {IDF_TARGET_NAME} (e.g., console commands), many PC Terminals wait for the {IDF_TARGET_NAME} to ingest the bytes before allowing you to send more data. This is in contrast to using a USB-to-Serial (UART) bridge chip, which always ingests the bytes and sends them to a (possibly not listening) {IDF_TARGET_NAME}.

.. note::

    In rare cases, it is possible that data sent from {IDF_TARGET_NAME} to the host gets 'stuck' in host memory. Sending more data will get it 'unstuck', but if the application does not send more data, depending on the driver, this data needs to be flushed to the host manually. The non-blocking (default) driver and the VFS implementation will flush automatically after a newline. The blocking (interrupt-based) driver will automatically flush when its transmit buffer becomes empty.

Sleep Mode Considerations
-------------------------

The USB Serial/JTAG controller and its associated USB PHY are driven by particular clocks (e.g., APB and USB PHY clock) and belong to a particular power domain (e.g., digital power domain). Thus, any change to the clock and power domains associated with the USB Serial/JTAG controller, such as entering different sleep modes, can affect the controller's operation.

Deep-sleep
^^^^^^^^^^

When entering Deep-sleep, both the USB Serial/JTAG controller and the USB PHY are powered off, leading to the USB PHY's D+ line no longer being pulled up. As a result:

- When entering Deep-sleep, the USB Serial/JTAG device appears disconnected from the host/PC (even if the USB cable is still physically connected).
- When exiting Deep-sleep, the USB Serial/JTAG device reconnects to the host/PC.

Light-sleep
^^^^^^^^^^^

.. only:: not SOC_USB_SERIAL_JTAG_SUPPORT_LIGHT_SLEEP

When entering Light-sleep, the APB and USB PHY clock are gated. Thus, the USB Serial/JTAG controller is no longer able to receive or respond to any USB transactions from the connected host (including periodic CDC Data IN transactions). As a result:

- when entering Light-sleep, the USB Serial/JTAG device is unresponsive to the host/PC's USB CDC driver. The host/PC may then report the USB Serial/JTAG device as disconnected or erroneous (even if the USB cable is still physically connected).
- when exiting Light-sleep, it is possible that the host/PC does not re-enumerate (i.e., reconnect) the USB Serial/JTAG device given that the USB PHY's D+ line remains pulled up state during Light-sleep. Users may need to physically disconnect and then reconnect the USB cable.

Automatic and Manual Sleep Entry
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If users enter sleep manually (via :cpp:func:`esp_light_sleep_start` or :cpp:func:`esp_deep_sleep_start`), users should be cognizant of the fact that USB Serial/JTAG controller does not work during sleep. ESP-IDF **does not add any safety check to reject entry to sleep** even if the USB Serial/JTAG controller is connected. In the case where sleep is entered while the USB Serial/JTAG controller is connected, the connection can be re-established by unplugging and re-plugging the USB cable.

If users enter sleep automatically (via :cpp:func:`esp_pm_configure`), enabling the :ref:`CONFIG_USJ_NO_AUTO_LS_ON_CONNECTION` option allows the {IDF_TARGET_NAME} to automatically detect whether the USB Serial/JTAG controller is currently connected to a host, and prevent automatic entry to sleep as long as the connection persists. However, note that this option increases power consumption.


Application Examples
====================

- :example:`peripherals/usb_serial_jtag/usb_serial_jtag_echo` demonstrates how to use the USB_SERIAL_JTAG interfaces to echo back any data received on it.
````

## File: docs/en/api-guides/wifi-expansion.rst
````
Wi-Fi Expansion
===============

:link_to_translation:`zh_CN:[中文]`

.. only:: not SOC_WIFI_SUPPORTED

  {IDF_TARGET_NAME} does not support Wi-Fi functionality natively, but it is possible to use the same Wi-Fi API and features using Wi-Fi expansion.

.. only:: SOC_WIFI_SUPPORTED

  {IDF_TARGET_NAME} does support Wi-Fi functionality natively, please refer to :doc:`wifi` documentation. Even though Wi-Fi is supported on {IDF_TARGET_NAME}, it is possible to expand it and use another instance of Wi-Fi expansion interfaces using `esp_wifi_remote <https://components.espressif.com/components/espressif/esp_wifi_remote>`_ component.


{IDF_TARGET_NAME} esp_wifi_remote
---------------------------------

The principle of Wi-Fi Expansion operation is to add another ESP32 series, Wi-Fi capable, target connected to the {IDF_TARGET_NAME} in a pre-defined way. Your project could then include the `esp_wifi_remote <https://components.espressif.com/components/espressif/esp_wifi_remote>`_ component using:

.. code:: bash

  idf.py add-dependency esp_wifi_remote


Please follow the instructions described in the component documentation which is linked in the above paragraph.

.. only:: not SOC_WIFI_SUPPORTED

  To explore the Wi-Fi Expansion functionality on {IDF_TARGET_NAME}, you can get started with this example: :idf_file:`examples/protocols/mqtt/tcp/README.md` and choose Wi-Fi connection in the project configuration menu.

.. only:: SOC_WIRELESS_HOST_SUPPORTED

  {IDF_TARGET_NAME} esp-extconn
  -----------------------------------------

  The principle of esp-extconn operation is to add another supported target series connected to the {IDF_TARGET_NAME} in a pre-defined way. Compared to the esp_wifi_remote approach, the target side can operate without flash, as the firmware is delivered by the hosted side. Your project could then include the `esp-extconn <https://components.espressif.com/components/espressif/esp-extconn>`_ component using:

  .. code:: bash

    idf.py add-dependency esp-extconn


  Please follow the instructions described in the `esp-extconn documentation <https://github.com/espressif/esp-extconn/blob/master/README.md>`_.

  To explore the esp-extconn functionality on {IDF_TARGET_NAME}, you can get started with this example: :idf_file:`examples/wifi/iperf/README.md` and choose Wi-Fi connection in the project configuration menu.
````

## File: docs/en/api-guides/wifi-security.rst
````
Wi-Fi Security
==============

:link_to_translation:`zh_CN:[中文]`

{IDF_TARGET_NAME} Wi-Fi Security Features
-----------------------------------------

- Support for Protected Management Frames (PMF)
- Support for WPA3-Personal
- Support for Opportunistic Wireless Encryption (OWE)

In addition to traditional security methods (WEP/WPA-TKIP/WPA2-CCMP), {IDF_TARGET_NAME} Wi-Fi supports state-of-the-art security protocols, namely Protected Management Frames (PMF), Wi-Fi Protected Access 3 and Enhanced Open™ based on Opportunistic Wireless Encryption. WPA3 provides better privacy and robustness against known attacks on traditional modes. Enhanced Open™ enhances the security and privacy of users connecting to open (public) Wireless Networks without authentication.

Protected Management Frames (PMF)
---------------------------------

Introduction
++++++++++++

In Wi-Fi, management frames such as beacons, probes, authentication/deauthentication, and association/disassociation are used by non-AP stations to scan and connect to an AP. Unlike data frames, these frames are sent unencrypted.

An attacker can use eavesdropping and packet injection to send spoofed authentication/deauthentication or association/disassociation frames at the right time, leading to attacks such as Denial-of-Service (DOS) and man-in-the-middle.

PMF provides protection against these attacks by encrypting unicast management frames and providing integrity checks for broadcast management frames. These include deauthentication, disassociation, and robust management frames. It also provides a Secure Association (SA) teardown mechanism to prevent spoofed association/authentication frames from disconnecting already connected clients.

There are three types of PMF configuration modes on both the station and AP sides:

 - PMF Optional
 - PMF Required
 - PMF Disabled

API & Usage
+++++++++++

{IDF_TARGET_NAME} supports PMF in both the station and SoftAP mode. For both, the default mode is PMF Optional. For even higher security, PMF Required mode can be enabled by setting the ``required`` flag in ``pmf_cfg`` while using the :cpp:func:`esp_wifi_set_config` API. This results in the device only connecting to a PMF-enabled device and rejecting others. PMF Optional can be disabled using :cpp:func:`esp_wifi_disable_pmf_config` API. If SoftAP is started in WPA3 or WPA2/WPA3 mixed mode, trying to disable PMF results in an error.

.. attention::

    ``capable`` flag in ``pmf_cfg`` is deprecated and set to ``true`` internally. This is to take the additional security benefit of PMF whenever possible.

Wi-Fi Enterprise
---------------------------------

Introduction
++++++++++++

Enterprise security is the secure authentication mechanism for enterprise wireless networks. It uses the RADIUS server for authentication of network users before connecting to the Access Point (AP). The authentication process is based on 802.1X policy and comes with different Extended Authentication Protocol (EAP) methods such as TLS, TTLS, PEAP, and EAP-FAST. RADIUS server authenticates the users based on their credentials (username and password), digital certificates, or both.

.. note::

  {IDF_TARGET_NAME} supports Wi-Fi Enterprise only in station mode.

{IDF_TARGET_NAME} supports **WPA2-Enterprise** and **WPA3-Enterprise**. WPA3-Enterprise builds upon the foundation of WPA2-Enterprise with the additional requirement of using Protected Management Frames (PMF) and server certificate validation on all WPA3 connections. **WPA3-Enterprise also offers an additional secure mode using 192-bit minimum-strength security protocols and cryptographic tools to better protect sensitive data.** The 192-bit security mode offered by WPA3-Enterprise ensures the right combination of cryptographic tools is used and sets a consistent baseline of security within a WPA3 network. WPA3-Enterprise 192-bit mode is only supported by modules having :c:macro:`SOC_WIFI_GCMP_SUPPORT` support. Enable :ref:`CONFIG_ESP_WIFI_SUITE_B_192` flag to support WPA3-Enterprise with 192-bit mode.

{IDF_TARGET_NAME} supports the following EAP methods:
  - EAP-TLS: This is a certificate-based method and only requires SSID and EAP-IDF.
  - PEAP: This is a Protected EAP method. Usernames and passwords are mandatory.
  - EAP-TTLS: This is a credential-based method. Only server authentication is mandatory while user authentication is optional. Username and Password are mandatory. It supports different Phase2 methods, such as:
     - PAP: Password Authentication Protocol.
     - CHAP: Challenge Handshake Authentication Protocol.
     - MSCHAP and MSCHAP-V2.
  - EAP-FAST: This is an authentication method based on Protected Access Credentials (PAC) which also uses identity and password. Currently, :ref:`CONFIG_ESP_WIFI_MBEDTLS_TLS_CLIENT` flag should be disabled to use this feature.

- :example:`wifi/wifi_eap_fast` demonstrates how to connect {IDF_TARGET_NAME} to an AP with Wi-Fi Enterprise authentication using EAP-FAST, including the installation of a CA certificate, setting user credentials, enabling Wi-Fi Enterprise mode, and handling connection to the AP.

- :example:`wifi/wifi_enterprise` demonstrates how to connect {IDF_TARGET_NAME} to an AP with Wi-Fi Enterprise authentication using other EAP methods, such as EAP-TLS, EAP-PEAP, EAP-TTLS. For details on generating certificates with OpenSSL commands and running the example, refer to :example_file:`wifi/wifi_enterprise/README.md`.

WPA3-Personal
-------------

Introduction
++++++++++++

Wi-Fi Protected Access-3 (WPA3) is a set of enhancements to Wi-Fi access security intended to replace the current WPA2 standard. It includes new features and capabilities that offer significantly better protection against different types of attacks. It improves upon WPA2-Personal in the following ways:

  - WPA3 uses Simultaneous Authentication of Equals (SAE), which is a password-authenticated key agreement method based on Diffie-Hellman key exchange. Unlike WPA2, the technology is resistant to offline-dictionary attacks, where the attacker attempts to determine a shared password based on a captured 4-way handshake without any further network interaction.
  - Disallows outdated protocols such as TKIP, which is susceptible to simple attacks like MIC key recovery attacks.
  - Mandates Protected Management Frames (PMF), which provides protection for unicast and multicast robust management frames which include Disassoc and Deauth frames. This means that the attacker cannot disrupt an established WPA3 session by sending forged Assoc frames to the AP or Deauth/Disassoc frames to the station.
  - Provides forward secrecy, which means the captured data cannot be decrypted even if the password is compromised after data transmission.

{IDF_TARGET_NAME} station also supports following additional Wi-Fi CERTIFIED WPA3™ features:

 - **Transition Disable** : WPA3 defines transition modes for client devices so that they can connect to a network even when some of the APs in that network do not support the strongest security mode. Client device implementations typically configure network profiles in a transition mode by default. However, such a client device could be subject to an active downgrade attack in which the attacker causes the client device to use a lower security mode in order to exploit a vulnerability with that mode. WPA3 has introduced the Transition Disable feature to mitigate such attacks, by enabling client devices to change from a transition mode to an "only" mode when connecting to a network, once that network indicates it fully supports the higher security mode. Enable :cpp:type:`transition_disable` in :cpp:type:`wifi_sta_config_t` to enable this feature for {IDF_TARGET_NAME} station.

 - **SAE PUBLIC-KEY (PK)** : As the password at small public networks is shared with multiple users, it may be relatively easy for an attacker to find out the password, which is sufficient to launch an evil twin attack. Such attacks are prevented by an extension to WPA3-Personal called SAE-PK. The SAE-PK authentication exchange is very similar to the regular SAE exchange, with the addition of a digital signature sent by the AP to the client device. The client device validates the public key asserted by the AP based on the password fingerprint and verifies the signature using the public key. So even if the attacker knows the password, it does not know the private key to generate a valid signature, and therefore the client device is protected against an evil twin attack. Enable :ref:`CONFIG_ESP_WIFI_ENABLE_SAE_PK` and :cpp:type:`sae_pk_mode` in :cpp:type:`wifi_sta_config_t` to add support of SAE PK for {IDF_TARGET_NAME} station.

 - **SAE PWE Methods**: {IDF_TARGET_NAME} station as well as SoftAP supports SAE Password Element derivation method `Hunting And Pecking` and `Hash to Element (H2E)`. H2E is computationally efficient as it uses fewer iterations than Hunt and Peck, and also it mitigates side-channel attacks. These can be configured using the parameter :cpp:type:`sae_pwe_h2e` from :cpp:type:`wifi_sta_config_t` and :cpp:type:`wifi_ap_config_t` for station and SoftAP respectively. Hunt and peck, H2E both can be enabled by using :cpp:enumerator:`WPA3_SAE_PWE_BOTH` configuration.

Please refer to the `Security <https://www.wi-fi.org/discover-wi-fi/security>`_ section of Wi-Fi Alliance's official website for further details.

Setting up WPA3 Personal with {IDF_TARGET_NAME}
+++++++++++++++++++++++++++++++++++++++++++++++

A configuration option :ref:`CONFIG_ESP_WIFI_ENABLE_WPA3_SAE` is provided to enable/disable WPA3 for the station. By default, it is kept enabled. If disabled, {IDF_TARGET_NAME} will not be able to establish a WPA3 connection. Also under the Wi-Fi component, a configuration option :ref:`CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT` is provided to enable/disable WPA3 for SoftAP. Additionally, since PMF is mandated by WPA3 protocol, PMF Optional is set by default for station and SoftAP. PMF Required can be configured using Wi-Fi configuration. For WPA3 SoftAP, PMF Required is mandatory and will be configured and stored in NVS implicitly if not specified by the user.

Refer to `Protected Management Frames (PMF)`_ on how to set this mode.

After configuring all required settings for the WPA3-Personal station, application developers need not worry about the underlying security mode of the AP. WPA3-Personal is now the highest supported protocol in terms of security, so it is automatically selected for the connection whenever available. For example, if an AP is configured to be in WPA3 Transition Mode, where it advertises as both WPA2 and WPA3 capable, the station chooses WPA3 for the connection with the above settings.

After configuring all required setting for WPA3-Personal SoftAP, application developers have to set ``WIFI_AUTH_WPA3_PSK`` for `authmode` in :cpp:type:`wifi_ap_config_t` to start AP in WPA3 security. SoftAP can be also configured to use ``WIFI_AUTH_WPA2_WPA3_PSK`` mixed mode.

Note that binary size will be increased by about 6.5 kilobytes after enabling :ref:`CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT`.

Wi-Fi Enhanced Open™
--------------------

Introduction
++++++++++++

Enhanced Open™ is used for providing security and privacy to users connecting to open (public) wireless networks, particularly in scenarios where user authentication is not desired or distribution of credentials impractical. Each user is provided with unique individual encryption keys that protect data exchange between a user device and the Wi-Fi network. Protected Management Frames further protect management traffic between the access point and the user device. Enhanced Open™ is based on the Opportunistic Wireless Encryption (OWE) standard. OWE Transition Mode enables a seamless transition from Open unencrypted WLANs to OWE WLANs without adversely impacting the end-user experience.

.. note::

  {IDF_TARGET_NAME} supports Wi-Fi Enhanced Open™ only in station mode.


Setting up OWE with {IDF_TARGET_NAME}
++++++++++++++++++++++++++++++++++++++

A configuration option :ref:`CONFIG_ESP_WIFI_ENABLE_WPA3_OWE_STA` and configuration parameter :cpp:type:`owe_enabled` in :cpp:type:`wifi_sta_config_t` is provided to enable OWE support for the station. To use OWE transition mode, along with the configuration provided above, `authmode` from :cpp:type:`wifi_scan_threshold_t` should be set to ``WIFI_AUTH_OPEN``.
````

## File: docs/en/api-guides/wifi.rst
````
Wi-Fi Driver
=============

:link_to_translation:`zh_CN:[中文]`

{IDF_TARGET_MAX_CONN_STA_NUM:default="15", esp32c2="4", esp32c3="10", esp32c6="10"}

{IDF_TARGET_SUB_MAX_NUM_FROM_KEYS:default="2", esp32c3="7", esp32c6="7"}

{IDF_TARGET_NAME} Wi-Fi Feature List
------------------------------------

The following features are supported:

.. only:: esp32 or esp32s2 or esp32c3 or esp32s3

    - 4 virtual Wi-Fi interfaces, which are STA, AP, Sniffer and reserved.
    - Station-only mode, AP-only mode, station/AP-coexistence mode
    - IEEE 802.11b, IEEE 802.11g, IEEE 802.11n, and APIs to configure the protocol mode
    - WPA/WPA2/WPA3/WPA2-Enterprise/WPA3-Enterprise/WAPI/WPS and DPP
    - AMSDU, AMPDU, HT40, QoS, and other key features
    - Modem-sleep
    - The Espressif-specific ESP-NOW protocol and Long Range mode, which supports up to **1 km** of data traffic
    - Up to 20 MBit/s TCP throughput and 30 MBit/s UDP throughput over the air
    - Sniffer
    - Both fast scan and all-channel scan
    - Multiple antennas
    - Channel state information

.. only:: esp32c2

    - 3 virtual Wi-Fi interfaces, which are STA, AP and Sniffer.
    - Station-only mode, AP-only mode, station/AP-coexistence mode
    - IEEE 802.11b, IEEE 802.11g, IEEE 802.11n, and APIs to configure the protocol mode
    - WPA/WPA2/WPA3/WPA2-Enterprise/WPA3-Enterprise/WPS and DPP
    - AMPDU, QoS, and other key features
    - Modem-sleep
    - Up to 20 MBit/s TCP throughput and 30 MBit/s UDP throughput over the air
    - Sniffer
    - Both fast scan and all-channel scan
    - Multiple antennas

.. only:: esp32c6

    - 4 virtual Wi-Fi interfaces, which are STA, AP, Sniffer and reserved.
    - Station-only mode, AP-only mode, station/AP-coexistence mode
    - IEEE 802.11b, IEEE 802.11g, IEEE 802.11n, IEEE 802.11ax, and APIs to configure the protocol mode
    - WPA/WPA2/WPA3/WPA2-Enterprise/WPA3-Enterprise/WAPI/WPS and DPP
    - AMSDU, AMPDU, HT40, QoS, and other key features
    - Modem-sleep
    - The Espressif-specific ESP-NOW protocol and Long Range mode, which supports up to **1 km** of data traffic
    - Up to 20 MBit/s TCP throughput and 30 MBit/s UDP throughput over the air
    - Sniffer
    - Both fast scan and all-channel scan
    - Multiple antennas
    - Channel state information
    - Individual TWT and Broadcast TWT
    - Downlink MU-MIMO
    - OFDMA
    - BSS Color

.. only:: esp32c5

    - 4 virtual Wi-Fi interfaces, which are STA, AP, Sniffer and reserved.
    - Station-only mode, AP-only mode, station/AP-coexistence mode
    - IEEE 802.11b, IEEE 802.11g, IEEE 802.11n, IEEE 802.11a, IEEE 802.11ac, IEEE 802.11ax, and APIs to configure the protocol mode
    - WPA/WPA2/WPA3/WPA2-Enterprise/WPA3-Enterprise/WAPI/WPS and DPP
    - AMSDU, AMPDU, HT40, QoS, and other key features
    - Modem-sleep
    - The Espressif-specific ESP-NOW protocol and Long Range mode (only supported on 2.4 GHz band), which supports up to **1 km** of data traffic
    - Up to 20 MBit/s TCP throughput and 30 MBit/s UDP throughput over the air
    - Sniffer
    - Both fast scan and all-channel scan
    - Multiple antennas
    - Channel state information
    - Individual TWT and Broadcast TWT
    - Downlink MU-MIMO
    - OFDMA
    - BSS Color
.. only:: SOC_WIFI_NAN_SUPPORT

    - Wi-Fi Aware (NAN)


How To Write a Wi-Fi Application
----------------------------------

Preparation
+++++++++++

Generally, the most effective way to begin your own Wi-Fi application is to select an example which is similar to your own application, and port the useful part into your project. It is not a MUST, but it is strongly recommended that you take some time to read this article first, especially if you want to program a robust Wi-Fi application.

This article is supplementary to the Wi-Fi APIs/Examples. It describes the principles of using the Wi-Fi APIs, the limitations of the current Wi-Fi API implementation, and the most common pitfalls in using Wi-Fi. This article also reveals some design details of the Wi-Fi driver. We recommend you to select an :example:`example <wifi>`.

- :example:`wifi/getting_started/station` demonstrates how to use the station functionality to connect to an AP.

- :example:`wifi/getting_started/softAP` demonstrates how to use the SoftAP functionality to configure {IDF_TARGET_NAME} as an AP.

- :example:`wifi/scan` demonstrates how to scan for available APs, configure the scan settings, and display the scan results.

- :example:`wifi/fast_scan` demonstrates how to perform fast and all channel scans for nearby APs, set thresholds for signal strength and authentication modes, and connect to the best fitting AP based on signal strength and authentication mode.

- :example:`wifi/wps` demonstrates how to use the WPS enrollee feature to simplify the process of connecting to a Wi-Fi router, with options for PIN or PBC modes.

- :example:`wifi/wps_softap_registrar` demonstrates how to use the WPS registrar feature on SoftAP mode, simplifying the process of connecting to a Wi-Fi SoftAP from a station.

- :example:`wifi/smart_config` demonstrates how to use the smartconfig feature to connect to a target AP using the ESPTOUCH app.

- :example:`wifi/power_save` demonstrates how to use the power save mode in station mode.

- :example:`wifi/softap_sta` demonstrates how to configure {IDF_TARGET_NAME} to function as both an AP and a station simultaneously, effectively enabling it to act as a Wi-Fi NAT router.

- :example:`wifi/iperf` demonstrates how to implement the protocol used by the iPerf performance measurement tool, allowing for performance measurement between two chips or between a single chip and a computer running the iPerf tool, with specific instructions for testing station/soft-AP TCP/UDP RX/TX throughput.

- :example:`wifi/roaming/roaming_app` demonstrates how to use the Wi-Fi Roaming App functionality to efficiently roam between compatible APs.

- :example:`wifi/roaming/roaming_11kvr` demonstrates how to implement roaming using 11k and 11v APIs.

.. only:: SOC_WIFI_HE_SUPPORT

    - :example:`wifi/itwt` demonstrates how to use the iTWT feature, which only works in station mode and under different power save modes, with commands for setup, teardown, and suspend, and also shows the difference in current consumption when iTWT is enabled or disabled.

Setting Wi-Fi Compile-time Options
++++++++++++++++++++++++++++++++++++

Refer to `Wi-Fi Menuconfig`_.

Init Wi-Fi
+++++++++++

Refer to `{IDF_TARGET_NAME} Wi-Fi station General Scenario`_ and `{IDF_TARGET_NAME} Wi-Fi AP General Scenario`_.

Start/Connect Wi-Fi
++++++++++++++++++++

Refer to `{IDF_TARGET_NAME} Wi-Fi station General Scenario`_ and `{IDF_TARGET_NAME} Wi-Fi AP General Scenario`_.

Event-Handling
++++++++++++++

Generally, it is easy to write code in "sunny-day" scenarios, such as `WIFI_EVENT_STA_START`_ and `WIFI_EVENT_STA_CONNECTED`_. The hard part is to write routines in "rainy-day" scenarios, such as `WIFI_EVENT_STA_DISCONNECTED`_. Good handling of "rainy-day" scenarios is fundamental to robust Wi-Fi applications. Refer to `{IDF_TARGET_NAME} Wi-Fi Event Description`_, `{IDF_TARGET_NAME} Wi-Fi station General Scenario`_, and `{IDF_TARGET_NAME} Wi-Fi AP General Scenario`_. See also the :doc:`overview of the Event Loop Library in ESP-IDF <../api-reference/system/esp_event>`.

Write Error-Recovery Routines Correctly at All Times
++++++++++++++++++++++++++++++++++++++++++++++++++++

Just like the handling of "rainy-day" scenarios, a good error-recovery routine is also fundamental to robust Wi-Fi applications. Refer to `{IDF_TARGET_NAME} Wi-Fi API Error Code`_.


{IDF_TARGET_NAME} Wi-Fi API Error Code
--------------------------------------

All of the {IDF_TARGET_NAME} Wi-Fi APIs have well-defined return values, namely, the error code. The error code can be categorized into:

 - No errors, e.g., :c:macro:`ESP_OK` means that the API returns successfully.
 - Recoverable errors, such as :c:macro:`ESP_ERR_NO_MEM`.
 - Non-recoverable, non-critical errors.
 - Non-recoverable, critical errors.

Whether the error is critical or not depends on the API and the application scenario, and it is defined by the API user.

**The primary principle to write a robust application with Wi-Fi API is to always check the error code and write the error-handling code.** Generally, the error-handling code can be used:

 - For recoverable errors, in which case you can write a recoverable-error code. For example, when :cpp:func:`esp_wifi_start()` returns :c:macro:`ESP_ERR_NO_MEM`, the recoverable-error code vTaskDelay can be called in order to get a microseconds' delay for another try.
 - For non-recoverable, yet non-critical errors, in which case printing the error code is a good method for error handling.
 - For non-recoverable and also critical errors, in which case "assert" may be a good method for error handling. For example, if :cpp:func:`esp_wifi_set_mode()` returns ``ESP_ERR_WIFI_NOT_INIT``, it means that the Wi-Fi driver is not initialized by :cpp:func:`esp_wifi_init()` successfully. You can detect this kind of error very quickly in the application development phase.

In :component_file:`esp_common/include/esp_err.h`, ``ESP_ERROR_CHECK`` checks the return values. It is a rather commonplace error-handling code and can be used as the default error-handling code in the application development phase. However, it is strongly recommended that API users write their own error-handling code.

{IDF_TARGET_NAME} Wi-Fi API Parameter Initialization
----------------------------------------------------

When initializing struct parameters for the API, one of two approaches should be followed:

- Explicitly set all fields of the parameter.
- Use get API to get current configuration first, then set application specific fields.

Initializing or getting the entire structure is very important, because most of the time the value 0 indicates that the default value is used. More fields may be added to the struct in the future and initializing these to zero ensures the application will still work correctly after ESP-IDF is updated to a new release.

.. _wifi-programming-model:

{IDF_TARGET_NAME} Wi-Fi Programming Model
-----------------------------------------

The {IDF_TARGET_NAME} Wi-Fi programming model is depicted as follows:

.. blockdiag::
    :caption: Wi-Fi Programming Model
    :align: center

    blockdiag wifi-programming-model {

        # global attributes
        node_height = 60;
        node_width = 100;
        span_width = 100;
        span_height = 60;
        default_shape = roundedbox;
        default_group_color = none;

        # node labels
        TCP_STACK [label="TCP\n stack", fontsize=12];
        EVNT_TASK [label="Event\n task", fontsize=12];
        APPL_TASK [label="Application\n task", width = 120, fontsize=12];
        WIFI_DRV  [label="Wi-Fi\n Driver", width = 120, fontsize=12];
        KNOT [shape=none];

        # node connections + labels
        TCP_STACK -> EVNT_TASK [label=event];
        EVNT_TASK -> APPL_TASK [label="callback\n or event"];

        # arrange nodes vertically
        group {
           label = "default handler";
           orientation = portrait;
           EVNT_TASK <- WIFI_DRV [label=event];
        }

        # intermediate node
        group {
            label = "user handler";
            orientation = portrait;
            APPL_TASK -- KNOT;
        }
        WIFI_DRV <- KNOT [label="API\n call"];
    }


The Wi-Fi driver can be considered a black box that knows nothing about high-layer code, such as the TCP/IP stack, application task, and event task. The application task (code) generally calls :doc:`Wi-Fi driver APIs <../api-reference/network/esp_wifi>` to initialize Wi-Fi and handles Wi-Fi events when necessary. Wi-Fi driver receives API calls, handles them, and posts events to the application.

Wi-Fi event handling is based on the :doc:`esp_event library <../api-reference/system/esp_event>`. Events are sent by the Wi-Fi driver to the :ref:`default event loop <esp-event-default-loops>`. Application may handle these events in callbacks registered using :cpp:func:`esp_event_handler_register()`. Wi-Fi events are also handled by :doc:`esp_netif component <../api-reference/network/esp_netif>` to provide a set of default behaviors. For example, when Wi-Fi station connects to an AP, esp_netif will automatically start the DHCP client by default.


{IDF_TARGET_NAME} Wi-Fi Event Description
-----------------------------------------

WIFI_EVENT_WIFI_READY
++++++++++++++++++++++++++++++++++++

The Wi-Fi driver will never generate this event, which, as a result, can be ignored by the application event callback. This event may be removed in future releases.

WIFI_EVENT_SCAN_DONE
++++++++++++++++++++++++++++++++++++

The scan-done event is triggered by :cpp:func:`esp_wifi_scan_start()` and will arise in the following scenarios:

  - The scan is completed, e.g., the target AP is found successfully, or all channels have been scanned.
  - The scan is stopped by :cpp:func:`esp_wifi_scan_stop()`.
  - The :cpp:func:`esp_wifi_scan_start()` is called before the scan is completed. A new scan will override the current scan and a scan-done event will be generated.

The scan-done event will not arise in the following scenarios:

  - It is a blocked scan.
  - The scan is caused by :cpp:func:`esp_wifi_connect()`.

Upon receiving this event, the event task does nothing. The application event callback needs to call :cpp:func:`esp_wifi_scan_get_ap_num()` and :cpp:func:`esp_wifi_scan_get_ap_records()` to fetch the scanned AP list and trigger the Wi-Fi driver to free the internal memory which is allocated during the scan **(do not forget to do this!)**.
Refer to `{IDF_TARGET_NAME} Wi-Fi Scan`_ for a more detailed description.

WIFI_EVENT_STA_START
++++++++++++++++++++++++++++++++++++

If :cpp:func:`esp_wifi_start()` returns :c:macro:`ESP_OK` and the current Wi-Fi mode is station or station/AP, then this event will arise. Upon receiving this event, the event task will initialize the LwIP network interface (netif). Generally, the application event callback needs to call :cpp:func:`esp_wifi_connect()` to connect to the configured AP.

WIFI_EVENT_STA_STOP
++++++++++++++++++++++++++++++++++++

If :cpp:func:`esp_wifi_stop()` returns :c:macro:`ESP_OK` and the current Wi-Fi mode is station or station/AP, then this event will arise. Upon receiving this event, the event task will release the station's IP address, stop the DHCP client, remove TCP/UDP-related connections, and clear the LwIP station netif, etc. The application event callback generally does not need to do anything.

WIFI_EVENT_STA_CONNECTED
++++++++++++++++++++++++++++++++++++

If :cpp:func:`esp_wifi_connect()` returns :c:macro:`ESP_OK` and the station successfully connects to the target AP, the connection event will arise. Upon receiving this event, the event task starts the DHCP client and begins the DHCP process of getting the IP address. Then, the Wi-Fi driver is ready for sending and receiving data. This moment is good for beginning the application work, provided that the application does not depend on LwIP, namely the IP address. However, if the application is LwIP-based, then you need to wait until the *got ip* event comes in.

WIFI_EVENT_STA_DISCONNECTED
++++++++++++++++++++++++++++++++++++

This event can be generated in the following scenarios:

  - When :cpp:func:`esp_wifi_disconnect()` or :cpp:func:`esp_wifi_stop()` is called and the station is already connected to the AP.
  - When :cpp:func:`esp_wifi_connect()` is called, but the Wi-Fi driver fails to set up a connection with the AP due to certain reasons, e.g., the scan fails to find the target AP or the authentication times out. If there are more than one AP with the same SSID, the disconnected event will be raised after the station fails to connect all of the found APs.
  - When the Wi-Fi connection is disrupted because of specific reasons, e.g., the station continuously loses N beacons, the AP kicks off the station, or the AP's authentication mode is changed.

Upon receiving this event, the default behaviors of the event task are:

- Shutting down the station's LwIP netif.
- Notifying the LwIP task to clear the UDP/TCP connections which cause the wrong status to all sockets. For socket-based applications, the application callback can choose to close all sockets and re-create them, if necessary, upon receiving this event.

The most common event handle code for this event in application is to call :cpp:func:`esp_wifi_connect()` to reconnect the Wi-Fi. However, if the event is raised because :cpp:func:`esp_wifi_disconnect()` is called, the application should not call :cpp:func:`esp_wifi_connect()` to reconnect. It is the application's responsibility to distinguish whether the event is caused by :cpp:func:`esp_wifi_disconnect()` or other reasons. Sometimes a better reconnection strategy is required. Refer to `Wi-Fi Reconnect`_ and `Scan When Wi-Fi Is Connecting`_.

Another thing that deserves attention is that the default behavior of LwIP is to abort all TCP socket connections on receiving the disconnect. In most cases, it is not a problem. However, for some special applications, this may not be what they want. Consider the following scenarios:

- The application creates a TCP connection to maintain the application-level keep-alive data that is sent out every 60 seconds.
- Due to certain reasons, the Wi-Fi connection is cut off, and the `WIFI_EVENT_STA_DISCONNECTED`_ is raised. According to the current implementation, all TCP connections will be removed and the keep-alive socket will be in a wrong status. However, since the application designer believes that the network layer should **ignore** this error at the Wi-Fi layer, the application does not close the socket.
- Five seconds later, the Wi-Fi connection is restored because :cpp:func:`esp_wifi_connect()` is called in the application event callback function. **Moreover, the station connects to the same AP and gets the same IPV4 address as before**.
- Sixty seconds later, when the application sends out data with the keep-alive socket, the socket returns an error and the application closes the socket and re-creates it when necessary.

In above scenarios, ideally, the application sockets and the network layer should not be affected, since the Wi-Fi connection only fails temporarily and recovers very quickly.

IP_EVENT_STA_GOT_IP
++++++++++++++++++++++++++++++++++++

This event arises when the DHCP client successfully gets the IPV4 address from the DHCP server, or when the IPV4 address is changed. The event means that everything is ready and the application can begin its tasks (e.g., creating sockets).

The IPV4 may be changed because of the following reasons:

  - The DHCP client fails to renew/rebind the IPV4 address, and the station's IPV4 is reset to 0.
  - The DHCP client rebinds to a different address.
  - The static-configured IPV4 address is changed.

Whether the IPV4 address is changed or not is indicated by the field ``ip_change`` of ``ip_event_got_ip_t``.

The socket is based on the IPV4 address, which means that, if the IPV4 changes, all sockets relating to this IPV4 will become abnormal. Upon receiving this event, the application needs to close all sockets and recreate the application when the IPV4 changes to a valid one.

IP_EVENT_GOT_IP6
++++++++++++++++++++++++++++++++++++

This event arises when the IPV6 SLAAC support auto-configures an address for the {IDF_TARGET_NAME}, or when this address changes. The event means that everything is ready and the application can begin its tasks, e.g., creating sockets.

IP_EVENT_STA_LOST_IP
++++++++++++++++++++++++++++++++++++

This event arises when the IPV4 address becomes invalid.

IP_EVENT_STA_LOST_IP does not arise immediately after the Wi-Fi disconnects. Instead, it starts an IPV4 address lost timer. If the IPV4 address is got before ip lost timer expires, IP_EVENT_STA_LOST_IP does not happen. Otherwise, the event arises when the IPV4 address lost timer expires.

Generally, the application can ignore this event, because it is just a debug event to inform that the IPV4 address is lost.

WIFI_EVENT_AP_START
++++++++++++++++++++++++++++++++++++

Similar to `WIFI_EVENT_STA_START`_.

WIFI_EVENT_AP_STOP
++++++++++++++++++++++++++++++++++++

Similar to `WIFI_EVENT_STA_STOP`_.

WIFI_EVENT_AP_STACONNECTED
++++++++++++++++++++++++++++++++++++

Every time a station is connected to {IDF_TARGET_NAME} AP, the `WIFI_EVENT_AP_STACONNECTED`_ will arise. Upon receiving this event, the event task will do nothing, and the application callback can also ignore it. However, you may want to do something, for example, to get the info of the connected STA.

WIFI_EVENT_AP_STADISCONNECTED
++++++++++++++++++++++++++++++++++++

This event can happen in the following scenarios:

  - The application calls :cpp:func:`esp_wifi_disconnect()`, or :cpp:func:`esp_wifi_deauth_sta()`, to manually disconnect the station.
  - The Wi-Fi driver kicks off the station, e.g., because the AP has not received any packets in the past five minutes. The time can be modified by :cpp:func:`esp_wifi_set_inactive_time()`.
  - The station kicks off the AP.

When this event happens, the event task will do nothing, but the application event callback needs to do something, e.g., close the socket which is related to this station.

WIFI_EVENT_AP_PROBEREQRECVED
++++++++++++++++++++++++++++++++++++

This event is disabled by default. The application can enable it via API :cpp:func:`esp_wifi_set_event_mask()`.
When this event is enabled, it will be raised each time the AP receives a probe request.

WIFI_EVENT_STA_BEACON_TIMEOUT
++++++++++++++++++++++++++++++++++++

If the station does not receive the beacon of the connected AP within the inactive time, the beacon timeout happens, the `WIFI_EVENT_STA_BEACON_TIMEOUT`_ will arise. The application can set inactive time via API :cpp:func:`esp_wifi_set_inactive_time()`.

WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The `WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START`_ will arise at the start of connectionless module `Interval`. See :ref:`connectionless module power save <connectionless-module-power-save>`.

{IDF_TARGET_NAME} Wi-Fi Station General Scenario
------------------------------------------------

Below is a "big scenario" which describes some small scenarios in station mode:

.. seqdiag::
    :caption: Sample Wi-Fi Event Scenarios in Station Mode
    :align: center

    seqdiag sample-scenarios-station-mode {
        activation = none;
        node_width = 80;
        node_height = 60;
        edge_length = 140;
        span_height = 5;
        default_shape = roundedbox;
        default_fontsize = 12;

        MAIN_TASK  [label = "Main\ntask"];
        APP_TASK   [label = "App\ntask"];
        EVENT_TASK [label = "Event\ntask"];
        LwIP_TASK  [label = "LwIP\ntask"];
        WIFI_TASK  [label = "Wi-Fi\ntask"];

        === 1. Init Phase ===
        MAIN_TASK  ->  LwIP_TASK   [label="1.1> Create / init LwIP"];
        MAIN_TASK  ->  EVENT_TASK  [label="1.2> Create / init event"];
        MAIN_TASK  ->  WIFI_TASK   [label="1.3> Create / init Wi-Fi"];
        MAIN_TASK  ->  APP_TASK    [label="1.4> Create app task"];
        === 2. Configure Phase ===
        MAIN_TASK  ->  WIFI_TASK   [label="2> Configure Wi-Fi"];
        === 3. Start Phase ===
        MAIN_TASK  ->  WIFI_TASK   [label="3.1> Start Wi-Fi"];
        EVENT_TASK <-  WIFI_TASK   [label="3.2> WIFI_EVENT_STA_START"];
        APP_TASK   <-  EVENT_TASK  [label="3.3> WIFI_EVENT_STA_START"];
        === 4. Connect Phase ===
        APP_TASK   ->  WIFI_TASK   [label="4.1> Connect Wi-Fi"];
        EVENT_TASK <-  WIFI_TASK   [label="4.2> WIFI_EVENT_STA_CONNECTED"];
        APP_TASK   <- EVENT_TASK   [label="4.3> WIFI_EVENT_STA_CONNECTED"];
        === 5. Got IP Phase ===
        EVENT_TASK ->  LwIP_TASK   [label="5.1> Start DHCP client"];
        EVENT_TASK <-  LwIP_TASK   [label="5.2> IP_EVENT_STA_GOT_IP"];
        APP_TASK   <-  EVENT_TASK  [label="5.3> IP_EVENT_STA_GOT_IP"];
        APP_TASK   ->  APP_TASK    [label="5.4> socket related init"];
        === 6. Disconnect Phase ===
        EVENT_TASK <-  WIFI_TASK   [label="6.1> WIFI_EVENT_STA_DISCONNECTED"];
        APP_TASK   <-  EVENT_TASK  [label="6.2> WIFI_EVENT_STA_DISCONNECTED"];
        APP_TASK   ->  APP_TASK    [label="6.3> disconnect handling"];
        === 7. IP Change Phase ===
        EVENT_TASK <-  LwIP_TASK   [label="7.1> IP_EVENT_STA_GOT_IP"];
        APP_TASK   <-  EVENT_TASK  [label="7.2> IP_EVENT_STA_GOT_IP"];
        APP_TASK   ->  APP_TASK    [label="7.3> Socket error handling"];
        === 8. Deinit Phase ===
        APP_TASK   ->  WIFI_TASK   [label="8.1> Disconnect Wi-Fi"];
        APP_TASK   ->  WIFI_TASK   [label="8.2> Stop Wi-Fi"];
        APP_TASK   ->  WIFI_TASK   [label="8.3> Deinit Wi-Fi"];
    }


1. Wi-Fi/LwIP Init Phase
++++++++++++++++++++++++++++++

 - s1.1: The main task calls :cpp:func:`esp_netif_init()` to create an LwIP core task and initialize LwIP-related work.

 - s1.2: The main task calls :cpp:func:`esp_event_loop_create()` to create a system Event task and initialize an application event's callback function. In the scenario above, the application event's callback function does nothing but relaying the event to the application task.

 - s1.3: The main task calls :cpp:func:`esp_netif_create_default_wifi_ap()` or :cpp:func:`esp_netif_create_default_wifi_sta()` to create default network interface instance binding station or AP with TCP/IP stack.

 - s1.4: The main task calls :cpp:func:`esp_wifi_init()` to create the Wi-Fi driver task and initialize the Wi-Fi driver.

 - s1.5: The main task calls OS API to create the application task.

Step 1.1 ~ 1.5 is a recommended sequence that initializes a Wi-Fi-/LwIP-based application. However, it is **NOT** a must-follow sequence, which means that you can create the application task in step 1.1 and put all other initialization in the application task. Moreover, you may not want to create the application task in the initialization phase if the application task depends on the sockets. Rather, you can defer the task creation until the IP is obtained.

2. Wi-Fi Configuration Phase
+++++++++++++++++++++++++++++++

Once the Wi-Fi driver is initialized, you can start configuring the Wi-Fi driver. In this scenario, the mode is station, so you may need to call :cpp:func:`esp_wifi_set_mode` (WIFI_MODE_STA) to configure the Wi-Fi mode as station. You can call other `esp_wifi_set_xxx` APIs to configure more settings, such as the protocol mode, the country code, and the bandwidth. Refer to `{IDF_TARGET_NAME} Wi-Fi Configuration`_.

Generally, the Wi-Fi driver should be configured before the Wi-Fi connection is set up. But this is **NOT** mandatory, which means that you can configure the Wi-Fi connection anytime, provided that the Wi-Fi driver is initialized successfully. However, if the configuration does not need to change after the Wi-Fi connection is set up, you should configure the Wi-Fi driver at this stage, because the configuration APIs (such as :cpp:func:`esp_wifi_set_protocol()`) will cause the Wi-Fi to reconnect, which may not be desirable.

If the Wi-Fi NVS flash is enabled by menuconfig, all Wi-Fi configuration in this phase, or later phases, will be stored into flash. When the board powers on/reboots, you do not need to configure the Wi-Fi driver from scratch. You only need to call ``esp_wifi_get_xxx`` APIs to fetch the configuration stored in flash previously. You can also configure the Wi-Fi driver if the previous configuration is not what you want.

3. Wi-Fi Start Phase
++++++++++++++++++++++++++++++++

 - s3.1: Call :cpp:func:`esp_wifi_start()` to start the Wi-Fi driver.
 - s3.2: The Wi-Fi driver posts `WIFI_EVENT_STA_START`_ to the event task; then, the event task will do some common things and will call the application event callback function.
 - s3.3: The application event callback function relays the `WIFI_EVENT_STA_START`_ to the application task. We recommend that you call :cpp:func:`esp_wifi_connect()`. However, you can also call :cpp:func:`esp_wifi_connect()` in other phrases after the `WIFI_EVENT_STA_START`_ arises.

4. Wi-Fi Connect Phase
+++++++++++++++++++++++++++++++++

 - s4.1: Once :cpp:func:`esp_wifi_connect()` is called, the Wi-Fi driver will start the internal scan/connection process.

 - s4.2: If the internal scan/connection process is successful, the `WIFI_EVENT_STA_CONNECTED`_ will be generated. In the event task, it starts the DHCP client, which will finally trigger the DHCP process.

 - s4.3: In the above-mentioned scenario, the application event callback will relay the event to the application task. Generally, the application needs to do nothing, and you can do whatever you want, e.g., print a log.

In step 4.2, the Wi-Fi connection may fail because, for example, the password is wrong, or the AP is not found. In a case like this, `WIFI_EVENT_STA_DISCONNECTED`_ will arise and the reason for such a failure will be provided. For handling events that disrupt Wi-Fi connection, please refer to phase 6.

5. Wi-Fi 'Got IP' Phase
+++++++++++++++++++++++++++++++++

 - s5.1: Once the DHCP client is initialized in step 4.2, the *got IP* phase will begin.
 - s5.2: If the IP address is successfully received from the DHCP server, then `IP_EVENT_STA_GOT_IP`_ will arise and the event task will perform common handling.
 - s5.3: In the application event callback, `IP_EVENT_STA_GOT_IP`_ is relayed to the application task. For LwIP-based applications, this event is very special and means that everything is ready for the application to begin its tasks, e.g., creating the TCP/UDP socket. A very common mistake is to initialize the socket before `IP_EVENT_STA_GOT_IP`_ is received. **DO NOT start the socket-related work before the IP is received.**

6. Wi-Fi Disconnect Phase
+++++++++++++++++++++++++++++++++

 - s6.1: When the Wi-Fi connection is disrupted, e.g., the AP is powered off or the RSSI is poor, `WIFI_EVENT_STA_DISCONNECTED`_ will arise. This event may also arise in phase 3. Here, the event task will notify the LwIP task to clear/remove all UDP/TCP connections. Then, all application sockets will be in a wrong status. In other words, no socket can work properly when this event happens.
 - s6.2: In the scenario described above, the application event callback function relays `WIFI_EVENT_STA_DISCONNECTED`_ to the application task. The recommended actions are: 1) call :cpp:func:`esp_wifi_connect()` to reconnect the Wi-Fi, 2) close all sockets, and 3) re-create them if necessary. For details, please refer to `WIFI_EVENT_STA_DISCONNECTED`_.

7. Wi-Fi IP Change Phase
++++++++++++++++++++++++++++++++++

 - s7.1: If the IP address is changed, the `IP_EVENT_STA_GOT_IP`_ will arise with "ip_change" set to true.
 - s7.2: **This event is important to the application. When it occurs, the timing is good for closing all created sockets and recreating them.**


8. Wi-Fi Deinit Phase
++++++++++++++++++++++++++++

 - s8.1: Call :cpp:func:`esp_wifi_disconnect()` to disconnect the Wi-Fi connectivity.
 - s8.2: Call :cpp:func:`esp_wifi_stop()` to stop the Wi-Fi driver.
 - s8.3: Call :cpp:func:`esp_wifi_deinit()` to unload the Wi-Fi driver.


{IDF_TARGET_NAME} Wi-Fi AP General Scenario
---------------------------------------------

Below is a "big scenario" which describes some small scenarios in AP mode:

 .. seqdiag::
    :caption: Sample Wi-Fi Event Scenarios in AP Mode
    :align: center

    seqdiag sample-scenarios-soft-ap-mode {
        activation = none;
        node_width = 80;
        node_height = 60;
        edge_length = 140;
        span_height = 5;
        default_shape = roundedbox;
        default_fontsize = 12;

        MAIN_TASK  [label = "Main\ntask"];
        APP_TASK   [label = "App\ntask"];
        EVENT_TASK [label = "Event\ntask"];
        LwIP_TASK  [label = "LwIP\ntask"];
        WIFI_TASK  [label = "Wi-Fi\ntask"];

        === 1. Init Phase ===
        MAIN_TASK  ->  LwIP_TASK   [label="1.1> Create / init LwIP"];
        MAIN_TASK  ->  EVENT_TASK  [label="1.2> Create / init event"];
        MAIN_TASK  ->  WIFI_TASK   [label="1.3> Create / init Wi-Fi"];
        MAIN_TASK  ->  APP_TASK    [label="1.4> Create app task"];
        === 2. Configure Phase ===
        MAIN_TASK  ->  WIFI_TASK   [label="2> Configure Wi-Fi"];
        === 3. Start Phase ===
        MAIN_TASK  ->  WIFI_TASK   [label="3.1> Start Wi-Fi"];
        EVENT_TASK <-  WIFI_TASK   [label="3.2> WIFI_EVENT_AP_START"];
        APP_TASK   <-  EVENT_TASK  [label="3.3> WIFI_EVENT_AP_START"];
        === 4. Connect Phase ===
        EVENT_TASK <-  WIFI_TASK   [label="4.1> WIFI_EVENT_AP_STACONNECTED"];
        APP_TASK   <- EVENT_TASK   [label="4.2> WIFI_EVENT_AP_STACONNECTED"];
        === 5. Disconnect Phase ===
        EVENT_TASK <-  WIFI_TASK   [label="5.1> WIFI_EVENT_AP_STADISCONNECTED"];
        APP_TASK   <-  EVENT_TASK  [label="5.2> WIFI_EVENT_AP_STADISCONNECTED"];
        APP_TASK   ->  APP_TASK    [label="5.3> disconnect handling"];
        === 6. Deinit Phase ===
        APP_TASK   ->  WIFI_TASK   [label="6.1> Disconnect Wi-Fi"];
        APP_TASK   ->  WIFI_TASK   [label="6.2> Stop Wi-Fi"];
        APP_TASK   ->  WIFI_TASK   [label="6.3> Deinit Wi-Fi"];
    }


{IDF_TARGET_NAME} Wi-Fi Scan
----------------------------

Currently, the :cpp:func:`esp_wifi_scan_start()` API is supported only in station or station/AP mode.

Scan Type
+++++++++++++++++++++++++

.. list-table::
   :header-rows: 1
   :widths: 15 50

   * - Mode
     - Description
   * - Active Scan
     - Scan by sending a probe request. The default scan is an active scan.
   * - Passive Scan
     - No probe request is sent out. Just switch to the specific channel and wait for a beacon. Application can enable it via the scan_type field of :cpp:type:`wifi_scan_config_t`.
   * - Foreground Scan
     - This scan is applicable when there is no Wi-Fi connection in station mode. Foreground or background scanning is controlled by the Wi-Fi driver and cannot be configured by the application.
   * - Background Scan
     - This scan is applicable when there is a Wi-Fi connection in station mode or in station/AP mode. Whether it is a foreground scan or background scan depends on the Wi-Fi driver and cannot be configured by the application.
   * - All-Channel Scan
     - It scans all of the channels. If the channel field of :cpp:type:`wifi_scan_config_t` is set to 0, it is an all-channel scan.
   * - Specific Channel Scan
     - It scans specific channels only. If the channel field of :cpp:type:`wifi_scan_config_t` set to 1-14, it is a specific-channel scan.

The scan modes in above table can be combined arbitrarily, so there are in total 8 different scans:

 - All-Channel Background Active Scan
 - All-Channel Background Passive Scan
 - All-Channel Foreground Active Scan
 - All-Channel Foreground Passive Scan
 - Specific-Channel Background Active Scan
 - Specific-Channel Background Passive Scan
 - Specific-Channel Foreground Active Scan
 - Specific-Channel Foreground Passive Scan

Scan Configuration
+++++++++++++++++++++++++++++++++++++++

The scan type and other per-scan attributes are configured by :cpp:func:`esp_wifi_scan_start()`. The table below provides a detailed description of :cpp:type:`wifi_scan_config_t`.

.. list-table::
   :header-rows: 1
   :widths: 15 50

   * - Field
     - Description
   * - ssid
     - If the SSID is not NULL, it is only the AP with the same SSID that can be scanned.
   * - bssid
     - If the BSSID is not NULL, it is only the AP with the same BSSID that can be scanned.
   * - channel
     - If “channel” is 0, there will be an all-channel scan; otherwise, there will be a specific-channel scan.
   * - show_hidden
     - If “show_hidden” is 0, the scan ignores the AP with a hidden SSID; otherwise, the scan considers the hidden AP a normal one.
   * - scan_type
     - If “scan_type” is WIFI_SCAN_TYPE_ACTIVE, the scan is “active”; otherwise, it is a “passive” one.
   * - scan_time
     - This field is used to control how long the scan dwells on each channel.

       For passive scans, scan_time.passive designates the dwell time for each channel.

       For active scans, dwell times for each channel are listed in the table below. Here, min is short for scan time.active.min and max is short for scan_time.active.max.

       - min=0, max=0: scan dwells on each channel for 120 ms.
       - min>0, max=0: scan dwells on each channel for 120 ms.
       - min=0, max>0: scan dwells on each channel for ``max`` ms.
       - min>0, max>0: the minimum time the scan dwells on each channel is ``min`` ms. If no AP is found during this time frame, the scan switches to the next channel. Otherwise, the scan dwells on the channel for ``max`` ms.

       If you want to improve the performance of the scan, you can try to modify these two parameters.


There are also some global scan attributes which are configured by API :cpp:func:`esp_wifi_set_config()`, refer to `Station Basic Configuration`_

Scan All APs on All Channels (Foreground)
+++++++++++++++++++++++++++++++++++++++++++++

Scenario:

.. seqdiag::
    :caption: Foreground Scan of all Wi-Fi Channels
    :align: center

    seqdiag foreground-scan-all-channels {
        activation = none;
        node_width = 80;
        node_height = 60;
        edge_length = 160;
        span_height = 5;
        default_shape = roundedbox;
        default_fontsize = 12;

        APP_TASK   [label = "App\ntask"];
        EVENT_TASK [label = "Event\ntask"];
        WIFI_TASK  [label = "Wi-Fi\ntask"];

        APP_TASK   ->  WIFI_TASK  [label="1.1 > Configure country code"];
        APP_TASK   ->  WIFI_TASK  [label="1.2 > Scan configuration"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.1 > Scan channel 1"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.2 > Scan channel 2"];
        WIFI_TASK  ->  WIFI_TASK  [label="..."];
        WIFI_TASK  ->  WIFI_TASK  [label="2.x > Scan channel N"];
        EVENT_TASK <-  WIFI_TASK  [label="3.1 > WIFI_EVENT_SCAN_DONE"];
        APP_TASK   <-  EVENT_TASK [label="3.2 > WIFI_EVENT_SCAN_DONE"];
    }


The scenario above describes an all-channel, foreground scan. The foreground scan can only occur in station mode where the station does not connect to any AP. Whether it is a foreground or background scan is totally determined by the Wi-Fi driver, and cannot be configured by the application.

Detailed scenario description:

Scan Configuration Phase
**************************

 - s1.1: Call :cpp:func:`esp_wifi_set_country()` to set the country info if the default country info is not what you want. Refer to `Wi-Fi Country Code`_.
 - s1.2: Call :cpp:func:`esp_wifi_scan_start()` to configure the scan. To do so, you can refer to `Scan Configuration`_. Since this is an all-channel scan, just set the SSID/BSSID/channel to 0.


Wi-Fi Driver's Internal Scan Phase
**************************************

 - s2.1: The Wi-Fi driver switches to channel 1. In this case, the scan type is WIFI_SCAN_TYPE_ACTIVE, and a probe request is broadcasted. Otherwise, the Wi-Fi will wait for a beacon from the APs. The Wi-Fi driver will stay in channel 1 for some time. The dwell time is configured in min/max time, with the default value being 120 ms.
 - s2.2: The Wi-Fi driver switches to channel 2 and performs the same operation as in step 2.1.
 - s2.3: The Wi-Fi driver scans the last channel N, where N is determined by the country code which is configured in step 1.1.

Scan-Done Event Handling Phase
*********************************

 - s3.1: When all channels are scanned, `WIFI_EVENT_SCAN_DONE`_ will arise.
 - s3.2: The application's event callback function notifies the application task that `WIFI_EVENT_SCAN_DONE`_ is received. :cpp:func:`esp_wifi_scan_get_ap_num()` is called to get the number of APs that have been found in this scan. Then, it allocates enough entries and calls :cpp:func:`esp_wifi_scan_get_ap_records()` to get the AP records. Please note that the AP records in the Wi-Fi driver will be freed once :cpp:func:`esp_wifi_scan_get_ap_records()` is called. Do not call :cpp:func:`esp_wifi_scan_get_ap_records()` twice for a single scan-done event. If :cpp:func:`esp_wifi_scan_get_ap_records()` is not called when the scan-done event occurs, the AP records allocated by the Wi-Fi driver will not be freed. So, make sure you call :cpp:func:`esp_wifi_scan_get_ap_records()`, yet only once.

Scan All APs on All Channels (Background)
++++++++++++++++++++++++++++++++++++++++++

Scenario:

.. seqdiag::
    :caption: Background Scan of all Wi-Fi Channels
    :align: center

    seqdiag background-scan-all-channels {
        activation = none;
        node_width = 80;
        node_height = 60;
        edge_length = 160;
        span_height = 5;
        default_shape = roundedbox;
        default_fontsize = 12;

        APP_TASK   [label = "App\ntask"];
        EVENT_TASK [label = "Event\ntask"];
        WIFI_TASK  [label = "Wi-Fi\ntask"];

        APP_TASK   ->  WIFI_TASK  [label="1.1 > Configure country code"];
        APP_TASK   ->  WIFI_TASK  [label="1.2 > Scan configuration"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.1 > Scan channel 1"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.2 > Back to home channel H"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.3 > Scan channel 2"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.4 > Back to home channel H"];
        WIFI_TASK  ->  WIFI_TASK  [label="..."];
        WIFI_TASK  ->  WIFI_TASK  [label="2.x-1 > Scan channel N"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.x > Back to home channel H"];
        EVENT_TASK <-  WIFI_TASK  [label="3.1 > WIFI_EVENT_SCAN_DONE"];
        APP_TASK   <-  EVENT_TASK [label="3.2 > WIFI_EVENT_SCAN_DONE"];
    }

The scenario above is an all-channel background scan. Compared to `Scan All APs on All Channels (Foreground)`_ , the difference in the all-channel background scan is that the Wi-Fi driver will scan the back-to-home channel for 30 ms before it switches to the next channel to give the Wi-Fi connection a chance to transmit/receive data.

Scan for Specific AP on All Channels
+++++++++++++++++++++++++++++++++++++++

Scenario:

.. seqdiag::
    :caption: Scan of specific Wi-Fi Channels
    :align: center

    seqdiag scan-specific-channels {
        activation = none;
        node_width = 80;
        node_height = 60;
        edge_length = 160;
        span_height = 5;
        default_shape = roundedbox;
        default_fontsize = 12;

        APP_TASK   [label = "App\ntask"];
        EVENT_TASK [label = "Event\ntask"];
        WIFI_TASK  [label = "Wi-Fi\ntask"];

        APP_TASK   ->  WIFI_TASK  [label="1.1 > Configure country code"];
        APP_TASK   ->  WIFI_TASK  [label="1.2 > Scan configuration"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.1 > Scan channel C1"];
        WIFI_TASK  ->  WIFI_TASK  [label="2.2 > Scan channel C2"];
        WIFI_TASK  ->  WIFI_TASK  [label="..."];
        WIFI_TASK  ->  WIFI_TASK  [label="2.x > Scan channel CN, or the AP is found"];
        EVENT_TASK <-  WIFI_TASK  [label="3.1 > WIFI_EVENT_SCAN_DONE"];
        APP_TASK   <-  EVENT_TASK [label="3.2 > WIFI_EVENT_SCAN_DONE"];
    }

This scan is similar to `Scan All APs on All Channels (Foreground)`_. The differences are:

 - s1.1: In step 1.2, the target AP will be configured to SSID/BSSID.
 - s2.1 ~ s2.N: Each time the Wi-Fi driver scans an AP, it will check whether it is a target AP or not. If the scan is ``WIFI_FAST_SCAN`` scan and the target AP is found, then the scan-done event will arise and scanning will end; otherwise, the scan will continue. Please note that the first scanned channel may not be channel 1, because the Wi-Fi driver optimizes the scanning sequence.

It is a possible situation that there are multiple APs that match the target AP info, e.g., two APs with the SSID of "ap" are scanned. In this case, if the scan is ``WIFI_FAST_SCAN``, then only the first scanned "ap" will be found. If the scan is ``WIFI_ALL_CHANNEL_SCAN``, both "ap" will be found and the station will connect the "ap" according to the configured strategy. Refer to `Station Basic Configuration`_.

You can scan a specific AP, or all of them, in any given channel. These two scenarios are very similar.

Scan in Wi-Fi Connect
+++++++++++++++++++++++++

When :cpp:func:`esp_wifi_connect()` is called, the Wi-Fi driver will try to scan the configured AP first. The scan in "Wi-Fi Connect" is the same as `Scan for Specific AP On All Channels`_, except that no scan-done event will be generated when the scan is completed. If the target AP is found, the Wi-Fi driver will start the Wi-Fi connection; otherwise, `WIFI_EVENT_STA_DISCONNECTED`_ will be generated. Refer to `Scan for Specific AP On All Channels`_.

Scan in Blocked Mode
++++++++++++++++++++

If the block parameter of :cpp:func:`esp_wifi_scan_start()` is true, then the scan is a blocked one, and the application task will be blocked until the scan is done. The blocked scan is similar to an unblocked one, except that no scan-done event will arise when the blocked scan is completed.

Parallel Scan
+++++++++++++

Two application tasks may call :cpp:func:`esp_wifi_scan_start()` at the same time, or the same application task calls :cpp:func:`esp_wifi_scan_start()` before it gets a scan-done event. Both scenarios can happen. **However, the Wi-Fi driver does not support multiple concurrent scans adequately. As a result, concurrent scans should be avoided.** Support for concurrent scan will be enhanced in future releases, as the {IDF_TARGET_NAME}'s Wi-Fi functionality improves continuously.

Scan When Wi-Fi Is Connecting
+++++++++++++++++++++++++++++++

The :cpp:func:`esp_wifi_scan_start()` fails immediately if the Wi-Fi is connecting, because the connecting has higher priority than the scan. If scan fails because of connecting, the recommended strategy is to delay for some time and retry scan again. The scan will succeed once the connecting is completed.

However, the retry/delay strategy may not work all the time. Considering the following scenarios:

- The station is connecting a non-existing AP or it connects the existing AP with a wrong password, it always raises the event `WIFI_EVENT_STA_DISCONNECTED`_.
- The application calls :cpp:func:`esp_wifi_connect()` to reconnect on receiving the disconnect event.
- Another application task, e.g., the console task, calls :cpp:func:`esp_wifi_scan_start()` to do scan, the scan always fails immediately because the station keeps connecting.
- When scan fails, the application simply delays for some time and retries the scan.

In the above scenarios, the scan will never succeed because the connecting is in process. So if the application supports similar scenario, it needs to implement a better reconnection strategy. For example:

- The application can choose to define a maximum continuous reconnection counter and stop reconnecting once the counter reaches the maximum.
- The application can choose to reconnect immediately in the first N continuous reconnection, then give a delay sometime and reconnect again.

The application can define its own reconnection strategy to avoid the scan starve to death. Refer to <`Wi-Fi Reconnect`_>.

{IDF_TARGET_NAME} Wi-Fi Station Connecting Scenario
---------------------------------------------------

This scenario depicts the case if only one target AP is found in the scan phase. For scenarios where more than one AP with the same SSID is found, refer to `{IDF_TARGET_NAME} Wi-Fi Station Connecting When Multiple APs Are Found`_.

Generally, the application can ignore the connecting process. Below is a brief introduction to the process for those who are really interested.

Scenario:

.. seqdiag::
    :caption: Wi-Fi Station Connecting Process
    :align: center

    seqdiag station-connecting-process {
        activation = none;
        node_width = 80;
        node_height = 60;
        edge_length = 160;
        span_height = 5;
        default_shape = roundedbox;
        default_fontsize = 12;

        EVENT_TASK  [label = "Event\ntask"];
        WIFI_TASK   [label = "Wi-Fi\ntask"];
        AP          [label = "AP"];

        === 1. Scan Phase ===
        WIFI_TASK  ->  WIFI_TASK [label="1.1 > Scan"];
        EVENT_TASK <-  WIFI_TASK [label="1.2 > WIFI_EVENT_STA_DISCONNECTED"];
        === 2. Auth Phase ===
        WIFI_TASK  ->  AP        [label="2.1 > Auth request"];
        EVENT_TASK <-  WIFI_TASK [label="2.2 > WIFI_EVENT_STA_DISCONNECTED"];
        WIFI_TASK  <-  AP        [label="2.3 > Auth response"];
        EVENT_TASK <-  WIFI_TASK [label="2.4 > WIFI_EVENT_STA_DISCONNECTED"];
        === 3. Assoc Phase ===
        WIFI_TASK  ->  AP        [label="3.1 > Assoc request"];
        EVENT_TASK <-  WIFI_TASK [label="3.2 > WIFI_EVENT_STA_DISCONNECTED"];
        WIFI_TASK  <-  AP        [label="3.3 > Assoc response"];
        EVENT_TASK <-  WIFI_TASK [label="3.4 > WIFI_EVENT_STA_DISCONNECTED"];
        === 4. 4-way Handshake Phase ===
        EVENT_TASK <-  WIFI_TASK [label="4.1 > WIFI_EVENT_STA_DISCONNECTED"];
        WIFI_TASK  <-  AP        [label="4.2 > 1/4 EAPOL"];
        WIFI_TASK  ->  AP        [label="4.3 > 2/4 EAPOL"];
        EVENT_TASK <-  WIFI_TASK [label="4.4 > WIFI_EVENT_STA_DISCONNECTED"];
        WIFI_TASK  <-  AP        [label="4.5 > 3/4 EAPOL"];
        WIFI_TASK  ->  AP        [label="4.6 > 4/4 EAPOL"];
        EVENT_TASK <-  WIFI_TASK [label="4.7 > WIFI_EVENT_STA_CONNECTED"];
    }


Scan Phase
+++++++++++++++++++++

 - s1.1: The Wi-Fi driver begins scanning in "Wi-Fi Connect". Refer to `Scan in Wi-Fi Connect`_ for more details.
 - s1.2: If the scan fails to find the target AP, `WIFI_EVENT_STA_DISCONNECTED`_ will arise and the reason code could either be ``WIFI_REASON_NO_AP_FOUND`` or ``WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY`` or ``WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD`` or ``WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD`` depending of the Station's configuration. Refer to `Wi-Fi Reason Code`_.

Auth Phase
+++++++++++++++++++++

 - s2.1: The authentication request packet is sent and the auth timer is enabled.
 - s2.2: If the authentication response packet is not received before the authentication timer times out, `WIFI_EVENT_STA_DISCONNECTED`_ will arise and the reason code will be ``WIFI_REASON_AUTH_EXPIRE``. Refer to `Wi-Fi Reason Code`_.
 - s2.3: The auth-response packet is received and the auth-timer is stopped.
 - s2.4: The AP rejects authentication in the response and `WIFI_EVENT_STA_DISCONNECTED`_ arises, while the reason code is ``WIFI_REASON_AUTH_FAIL`` or the reasons specified by the AP. Refer to `Wi-Fi Reason Code`_.

Association Phase
+++++++++++++++++++++

 - s3.1: The association request is sent and the association timer is enabled.
 - s3.2: If the association response is not received before the association timer times out, `WIFI_EVENT_STA_DISCONNECTED`_ will arise and the reason code will be ``WIFI_REASON_DISASSOC_DUE_TO_INACTIVITY``. Refer to `Wi-Fi Reason Code`_.
 - s3.3: The association response is received and the association timer is stopped.
 - s3.4: The AP rejects the association in the response and `WIFI_EVENT_STA_DISCONNECTED`_ arises, while the reason code is the one specified in the association response. Refer to `Wi-Fi Reason Code`_.


Four-way Handshake Phase
++++++++++++++++++++++++++

 - s4.1: The handshake timer is enabled, and the 1/4 EAPOL is not received before the handshake timer expires. `WIFI_EVENT_STA_DISCONNECTED`_ will arise and the reason code will be ``WIFI_REASON_HANDSHAKE_TIMEOUT``. Refer to `Wi-Fi Reason Code`_.
 - s4.2: The 1/4 EAPOL is received.
 - s4.3: The station replies 2/4 EAPOL.
 - s4.4: If the 3/4 EAPOL is not received before the handshake timer expires, `WIFI_EVENT_STA_DISCONNECTED`_ will arise and the reason code will be ``WIFI_REASON_HANDSHAKE_TIMEOUT``. Refer to `Wi-Fi Reason Code`_.
 - s4.5: The 3/4 EAPOL is received.
 - s4.6: The station replies 4/4 EAPOL.
 - s4.7: The station raises `WIFI_EVENT_STA_CONNECTED`_.


.. _esp_wifi_reason_code:

Wi-Fi Reason Code
+++++++++++++++++++++

The table below shows the reason-code defined in {IDF_TARGET_NAME}. The first column is the macro name defined in :component_file:`esp_wifi/include/esp_wifi_types.h`. The common prefix ``WIFI_REASON`` is removed, which means that ``UNSPECIFIED`` actually stands for ``WIFI_REASON_UNSPECIFIED`` and so on. The second column is the value of the reason. This reason value is same as defined in section 9.4.1.7 of IEEE 802.11-2020. (For more information, refer to the standard mentioned above.) The last column describes the reason. Reason-codes starting from 200 are Espressif defined reason-codes and are not part of IEEE 802.11-2020.\

Also note that REASON_NO_AP_FOUND_XXX codes are mentioned in increasing order of importance. So if a single AP has a combination of the above reasons for failure, the more important one will be reported. Additionally, if there are multiple APs that satisfy the identifying criteria and connecting to all of them fails for different reasons mentioned above, then the reason code reported is for the AP that failed connection due to the least important reason code, as it was the one closest to a successful connection.\

Following reason codes are renamed to their shorter form to wrap the table in page width.

- TRANSMISSION_LINK_ESTABLISHMENT_FAILED : TX_LINK_EST_FAILED
- NO_AP_FOUND_W_COMPATIBLE_SECURITY : NO_AP_FOUND_SECURITY
- NO_AP_FOUND_IN_AUTHMODE_THRESHOLD : NO_AP_FOUND_AUTHMODE
- NO_AP_FOUND_IN_RSSI_THRESHOLD : NO_AP_FOUND_RSSI

.. list-table::
   :header-rows: 1
   :widths: 41 10 49
   :class: longtable

   * - Reason code
     - Value
     - Description
   * - UNSPECIFIED
     - 1
     - Generally, it means an internal failure, e.g., the memory runs out, the internal TX fails, or the reason is received from the remote side.
   * - AUTH_EXPIRE
     - 2
     - The previous authentication is no longer valid.

       For the ESP station, this reason is reported when:

       - auth is timed out.
       - the reason is received from the AP.

       For the ESP AP, this reason is reported when:

       - the AP has not received any packets from the station in the past five minutes.
       - the AP is stopped by calling :cpp:func:`esp_wifi_stop()`.
       - the station is de-authed by calling :cpp:func:`esp_wifi_deauth_sta()`.
   * - AUTH_LEAVE
     - 3
     - De-authenticated, because the sending station is leaving (or has left).

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - DISASSOC_DUE_TO_INACTIVITY
     - 4
     - Disassociated due to inactivity.

       For the ESP station, this reason is reported when:

       - assoc is timed out.
       - it is received from the AP.

   * - ASSOC_TOOMANY
     - 5
     - Disassociated, because the AP is unable to handle all currently associated STAs at the same time.

       For the ESP station, this reason is reported when:

       - it is received from the AP.

       For the ESP AP, this reason is reported when:

       - the stations associated with the AP reach the maximum number that the AP can support.
   * - CLASS2_FRAME_FROM_NONAUTH_STA
     - 6
     - Class-2 frame received from a non-authenticated STA.

       For the ESP station, this reason is reported when:

       - it is received from the AP.

       For the ESP AP, this reason is reported when:

       - the AP receives a packet with data from a non-authenticated station.
   * - CLASS3_FRAME_FROM_NONASSOC_STA
     - 7
     - Class-3 frame received from a non-associated STA.

       For the ESP station, this reason is reported when:

       - it is received from the AP.

       For the ESP AP, this reason is reported when:

       - the AP receives a packet with data from a non-associated station.
   * - ASSOC_LEAVE
     - 8
     - Disassociated, because the sending station is leaving (or has left) BSS.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
       - the station is disconnected by :cpp:func:`esp_wifi_disconnect()` and other APIs.
   * - ASSOC_NOT_AUTHED
     - 9
     - station requesting (re)association is not authenticated by the responding STA.

       For the ESP station, this reason is reported when:

       - it is received from the AP.

       For the ESP AP, this reason is reported when:

       - the AP receives packets with data from an associated, yet not authenticated, station.
   * - DISASSOC_PWRCAP_BAD
     - 10
     - Disassociated, because the information in the Power Capability element is unacceptable.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - DISASSOC_SUPCHAN_BAD
     - 11
     - Disassociated, because the information in the Supported Channels element is unacceptable.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - BSS_TRANSITION_DISASSOC
     - 12
     - AP wants us to move to another AP, sent as a part of BTM procedure. Please note that when station is sending BTM request and moving to another AP, ROAMING reason code will be reported instead of this.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - IE_INVALID
     - 13
     - Invalid element, i.e., an element whose content does not meet the specifications of the Standard in frame formats clause.

       For the ESP station, this reason is reported when:

       - it is received from the AP.

       For the ESP AP, this reason is reported when:

       - the AP parses a wrong WPA or RSN IE.
   * - MIC_FAILURE
     - 14
     - Message integrity code (MIC) failure.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - 4WAY_HANDSHAKE_TIMEOUT
     - 15
     - Four-way handshake times out. For legacy reasons, in ESP this reason code is replaced with ``WIFI_REASON_HANDSHAKE_TIMEOUT``.

       For the ESP station, this reason is reported when:

       - the handshake times out.
       - it is received from the AP.
   * - GROUP_KEY_UPDATE_TIMEOUT
     - 16
     - Group-Key Handshake times out.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - IE_IN_4WAY_DIFFERS
     - 17
     - The element in the four-way handshake is different from the (Re-)Association Request/Probe and Response/Beacon frame.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
       - the station finds that the four-way handshake IE differs from the IE in the (Re-)Association Request/Probe and Response/Beacon frame.
   * - GROUP_CIPHER_INVALID
     - 18
     - Invalid group cipher.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - PAIRWISE_CIPHER_INVALID
     - 19
     - Invalid pairwise cipher.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - AKMP_INVALID
     - 20
     - Invalid AKMP.

       For the ESP station, this reason is reported when:
       - it is received from the AP.
   * - UNSUPP_RSN_IE_VERSION
     - 21
     - Unsupported RSNE version.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - INVALID_RSN_IE_CAP
     - 22
     - Invalid RSNE capabilities.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - 802_1X_AUTH_FAILED
     - 23
     - IEEE 802.1X. authentication failed.

       For the ESP station, this reason is reported when:

       - it is received from the AP.

       For the ESP AP, this reason is reported when:

       - IEEE 802.1X. authentication fails.
   * - CIPHER_SUITE_REJECTED
     - 24
     - Cipher suite rejected due to security policies.

       For the ESP station, this reason is reported when:

       - it is received from the AP.
   * - TDLS_PEER_UNREACHABLE
     - 25
     - TDLS direct-link teardown due to TDLS peer STA unreachable via the TDLS direct link.
   * - TDLS_UNSPECIFIED
     - 26
     - TDLS direct-link teardown for unspecified reason.
   * - SSP_REQUESTED_DISASSOC
     - 27
     - Disassociated because session terminated by SSP request.
   * - NO_SSP_ROAMING_AGREEMENT
     - 28
     - Disassociated because of lack of SSP roaming agreement.
   * - BAD_CIPHER_OR_AKM
     - 29
     - Requested service rejected because of SSP cipher suite or AKM requirement.
   * - NOT_AUTHORIZED_THIS_LOCATION
     - 30
     - Requested service not authorized in this location.
   * - SERVICE_CHANGE_PRECLUDES_TS
     - 31
     - TS deleted because QoS AP lacks sufficient bandwidth for this QoS STA due to a change in BSS service characteristics or operational mode (e.g., an HT BSS change from 40 MHz channel to 20 MHz channel).
   * - UNSPECIFIED_QOS
     - 32
     - Disassociated for unspecified, QoS-related reason.
   * - NOT_ENOUGH_BANDWIDTH
     - 33
     - Disassociated because QoS AP lacks sufficient bandwidth for this QoS STA.
   * - MISSING_ACKS
     - 34
     - Disassociated because excessive number of frames need to be acknowledged, but are not acknowledged due to AP transmissions and/or poor channel conditions.
   * - EXCEEDED_TXOP
     - 35
     - Disassociated because STA is transmitting outside the limits of its TXOPs.
   * - STA_LEAVING
     - 36
     - Requesting STA is leaving the BSS (or resetting).
   * - END_BA
     - 37
     - Requesting STA is no longer using the stream or session.
   * - UNKNOWN_BA
     - 38
     - Requesting STA received frames using a mechanism for which a setup has not been completed.
   * - TIMEOUT
     - 39
     - Requested from peer STA due to timeout
   * - Reserved
     - 40 ~ 45
     - Reserved as per IEEE80211-2020 specifications.
   * - PEER_INITIATED
     - 46
     - In a Disassociation frame: Disassociated because authorized access limit reached.
   * - AP_INITIATED
     - 47
     - In a Disassociation frame: Disassociated due to external service requirements.
   * - INVALID_FT_ACTION_FRAME_COUNT
     - 48
     - Invalid FT Action frame count.
   * - INVALID_PMKID
     - 49
     - Invalid pairwise master key identifier (PMKID).
   * - INVALID_MDE
     - 50
     - Invalid MDE.
   * - INVALID_FTE
     - 51
     - Invalid FTE
   * - TX_LINK_EST_FAILED
     - 67
     - TRANSMISSION_LINK_ESTABLISHMENT_FAILED will be reported when Transmission link establishment in alternative channel failed.
   * - ALTERATIVE_CHANNEL_OCCUPIED
     - 68
     - The alternative channel is occupied.
   * - BEACON_TIMEOUT
     - 200
     - Espressif-specific Wi-Fi reason code: when the station loses N beacons continuously, it will disrupt the connection and report this reason.
   * - NO_AP_FOUND
     - 201
     - Espressif-specific Wi-Fi reason code: when the station fails to scan the target AP, this reason code will be reported. In case of security mismatch or station's configuration mismatch, new reason codes NO_AP_FOUND_XXX will be reported.
   * - AUTH_FAIL
     - 202
     - Espressif-specific Wi-Fi reason code: the authentication fails, but not because of a timeout.
   * - ASSOC_FAIL
     - 203
     - Espressif-specific Wi-Fi reason code: the association fails, but not because of DISASSOC_DUE_TO_INACTIVITY or ASSOC_TOOMANY.
   * - HANDSHAKE_TIMEOUT
     - 204
     - Espressif-specific Wi-Fi reason code: the handshake fails for the same reason as that in WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT.
   * - CONNECTION_FAIL
     - 205
     - Espressif-specific Wi-Fi reason code: the connection to the AP has failed.
   * - AP_TSF_RESET
     - 206
     - Espressif-specific Wi-Fi reason code: the disconnection happened due to AP's TSF reset.
   * - ROAMING
     - 207
     - Espressif-specific Wi-Fi reason code: the station is roaming to another AP, this reason code is just for info, station will automatically move to another AP.
   * - ASSOC_COMEBACK_TIME_TOO_LONG
     - 208
     - Espressif-specific Wi-Fi reason code: This reason code will be reported when Assoc comeback time in association response is too high.
   * - SA_QUERY_TIMEOUT
     - 209
     - Espressif-specific Wi-Fi reason code: This reason code will be reported when AP did not reply of SA query sent by ESP station.
   * - NO_AP_FOUND_SECURITY
     - 210
     - Espressif-specific Wi-Fi reason code: NO_AP_FOUND_W_COMPATIBLE_SECURITY will be reported if an AP that fits identifying criteria (e.g. ssid) is found but the connection is rejected due to incompatible security configuration. These situations could be:

       - The Access Point is offering WEP security, but our station's password is not WEP-compliant.
       - The station is configured in Open mode; however, the Access Point is broadcasting in secure mode.
       - The Access Point uses Enterprise security, but we haven't set up the corresponding enterprise configuration, and vice versa.
       - SAE-PK is configured in the station configuration, but the Access Point does not support SAE-PK.
       - SAE-H2E is configured in the station configuration; however, the AP only supports WPA3-PSK or WPA3-WPA2-PSK.
       - The station is configured in secure mode (Password or Enterprise mode); however, an Open AP is found during the scan.
       - SAE HnP is configured in the station configuration; however, the AP supports H2E only.
       - H2E is disabled in the station configuration; however, the AP is WPA3-EXT-PSK, which requires H2E support.
       - The Access Point requires PMF, but the station is not configured for PMF capable/required.
       - The station configuration requires PMF, but the AP is not configured for PMF capable/required.
       - The Access Point is using unsupported group management/pairwise ciphers.
       - OWE is not enabled in the station configuration, but the discovered AP is using OWE only mode.
       - The Access Point is broadcasting an invalid RSNXE in its beacons.
       - The Access Point is in Independent BSS mode.

   * - NO_AP_FOUND_AUTHMODE
     - 211
     - Espressif-specific Wi-Fi reason code: NO_AP_FOUND_IN_AUTHMODE_THRESHOLD will be reported if an AP that fit identifying criteria (e.g. ssid) is found but the authmode threhsold set in the wifi_config_t is not met.
   * - NO_AP_FOUND_RSSI
     - 212
     - Espressif-specific Wi-Fi reason code: NO_AP_FOUND_IN_RSSI_THRESHOLD will be reported if an AP that fits identifying criteria (e.g. ssid) is found but the RSSI threhsold set in the wifi_config_t is not met.

Wi-Fi Reason code related to wrong password
++++++++++++++++++++++++++++++++++++++++++++++

The table below shows the Wi-Fi reason-code may related to wrong password.

.. list-table::
   :header-rows: 1
   :widths: 5 10 40

   * - Reason code
     - Value
     - Description
   * - 4WAY_HANDSHAKE_TIMEOUT
     - 15
     - Four-way handshake times out. Setting wrong password when STA connecting to an encrypted AP.
   * - NO_AP_FOUND
     - 201
     - This may related to wrong password in the two scenarios:

       - Setting password when STA connecting to an unencrypted AP.
       - Does not set password when STA connecting to an encrypted AP.
   * - HANDSHAKE_TIMEOUT
     - 204
     - Four-way handshake fails.

Wi-Fi Reason code related to low RSSI
++++++++++++++++++++++++++++++++++++++++++++++

The table below shows the Wi-Fi reason-code may related to low RSSI.

.. list-table::
   :header-rows: 1
   :widths: 5 10 40

   * - Reason code
     - Value
     - Description
   * - NO_AP_FOUND_IN_RSSI_THRESHOLD
     - 212
     - The station fails to scan the target AP due to low RSSI
   * - HANDSHAKE_TIMEOUT
     - 204
     - Four-way handshake fails.


{IDF_TARGET_NAME} Wi-Fi Station Connecting When Multiple APs Are Found
----------------------------------------------------------------------

This scenario is similar as `{IDF_TARGET_NAME} Wi-Fi Station Connecting Scenario`_. The difference is that the station will not raise the event `WIFI_EVENT_STA_DISCONNECTED`_ unless it fails to connect all of the found APs.

Wi-Fi Reconnect
---------------------------

The station may disconnect due to many reasons, e.g., the connected AP is restarted. It is the application's responsibility to reconnect. The recommended reconnection strategy is to call :cpp:func:`esp_wifi_connect()` on receiving event `WIFI_EVENT_STA_DISCONNECTED`_.

Sometimes the application needs more complex reconnection strategy:

- If the disconnect event is raised because the :cpp:func:`esp_wifi_disconnect()` is called, the application may not want to do the reconnection.
- If the :cpp:func:`esp_wifi_scan_start()` may be called at anytime, a better reconnection strategy is necessary. Refer to `Scan When Wi-Fi Is Connecting`_.

Another thing that need to be considered is that the reconnection may not connect the same AP if there are more than one APs with the same SSID. The reconnection always select current best APs to connect.

Wi-Fi Beacon Timeout
---------------------------

The beacon timeout mechanism is used by {IDF_TARGET_NAME} station to detect whether the AP is alive or not. If the station does not receive the beacon of the connected AP within the inactive time, the beacon timeout happens. The application can set inactive time via API :cpp:func:`esp_wifi_set_inactive_time()`.

After the beacon times out, the station sends 5 probe requests to the AP. If still no probe response or beacon is received from AP, the station disconnects from the AP and raises the event `WIFI_EVENT_STA_DISCONNECTED`_.

It should be considered that the timer used for beacon timeout will be reset during the scanning process. It means that the scan process will affect the triggering of the event `WIFI_EVENT_STA_BEACON_TIMEOUT`_.

{IDF_TARGET_NAME} Wi-Fi Configuration
-------------------------------------

All configurations will be stored into flash when the Wi-Fi NVS is enabled; otherwise, refer to `Wi-Fi NVS Flash`_.

Wi-Fi Mode
+++++++++++++++++++++++++
Call :cpp:func:`esp_wifi_set_mode()` to set the Wi-Fi mode.

.. list-table::
   :header-rows: 1
   :widths: 15 50

   * - Mode
     - Description
   * - ``WIFI_MODE_NULL``
     - NULL mode: in this mode, the internal data struct is not allocated to the station and the AP, while both the station and AP interfaces are not initialized for RX/TX Wi-Fi data. Generally, this mode is used for Sniffer, or when you only want to stop both the station and the AP without calling :cpp:func:`esp_wifi_deinit()` to unload the whole Wi-Fi driver.
   * - ``WIFI_MODE_STA``
     - Station mode: in this mode, :cpp:func:`esp_wifi_start()` will init the internal station data, while the station’s interface is ready for the RX and TX Wi-Fi data. After :cpp:func:`esp_wifi_connect()`, the station will connect to the target AP.
   * - ``WIFI_MODE_AP``
     - AP mode: in this mode, :cpp:func:`esp_wifi_start()` will init the internal AP data, while the AP’s interface is ready for RX/TX Wi-Fi data. Then, the Wi-Fi driver starts broad-casting beacons, and the AP is ready to get connected to other stations.
   * - ``WIFI_MODE_APSTA``
     - Station/AP coexistence mode: in this mode, :cpp:func:`esp_wifi_start()` will simultaneously initialize both the station and the AP. This is done in station mode and AP mode. Please note that the channel of the external AP, which the ESP station is connected to, has higher priority over the ESP AP channel.

.. only:: esp32c5

    Wi-Fi Band Mode Configuration
    ++++++++++++++++++++++++++++++

    The Wi-Fi band mode used by {IDF_TARGET_NAME} can be set via the function :cpp:func:`esp_wifi_set_band_mode()`.

    .. list-table::
        :header-rows: 1
        :widths: 15 50

        * - Mode
          - Description
        * - ``WIFI_BAND_MODE_2G_ONLY``
          - **2.4 GHz band mode**: The device operates only on 2.4 GHz band channels.
        * - ``WIFI_BAND_MODE_5G_ONLY``
          - **5 GHz band mode**: The device operates only on 5 GHz band channels.
        * - ``WIFI_BAND_MODE_AUTO``
          - **2.4 GHz + 5 GHz auto mode**: The device automatically selects either the 2.4 GHz or 5 GHz band based on the connected AP or SoftAP configuration.

    .. note::
        - ``WIFI_BAND_MODE_AUTO`` does not mean simultaneous dual-band support; it only allows automatic band selection.

        When operating in ``WIFI_BAND_MODE_AUTO`` mode, protocols and bandwidth can be configured separately for the 2.4 GHz and 5 GHz bands:

        - Use the function :cpp:func:`esp_wifi_set_protocols()` to set the supported protocol types for each band (e.g., 802.11b/g/n/ac/ax);

        - Use the function :cpp:func:`esp_wifi_set_bandwidths()` to set the bandwidth for each band (e.g., 20 MHz, 40 MHz).

.. only:: esp32c5

    AP Choose
    +++++++++++++++++++++++++

    When the device scans multiple APs with the same SSID, {IDF_TARGET_NAME} selects the most suitable AP to connect to based on signal strength (RSSI) and band information. The default policy usually prefers the AP with higher RSSI; however, in environments where 2.4 GHz and 5 GHz coexist, this can cause the device to favor the 2.4 GHz band, ignoring the performance benefits of the 5 GHz band.

    To address this, ESP-IDF provides the field :cpp:member:`rssi_5g_adjustment` in the :cpp:type:`wifi_scan_threshold_t` structure to optimize the priority of selecting 5 GHz APs.

    .. list-table::
      :header-rows: 1
      :widths: 30 70

      * - Field
        - Description
      * - ``rssi_5g_adjustment``
        - Used to adjust priority between 2.4 GHz and 5 GHz APs with the same SSID. The default value is ``10``, meaning when the 5 GHz AP's RSSI is within 10 dB lower than the 2.4 GHz AP, the 5 GHz AP will be preferred. Properly setting this parameter helps the device prioritize 5 GHz networks that offer better bandwidth and interference resistance when signal strengths are close.

    Example:

    Suppose the device scans the following two APs with the SSID "MyWiFi":

    - 2.4 GHz AP: RSSI = -60 dBm
    - 5 GHz AP: RSSI = -68 dBm

    Since ``rssi_5g_adjustment = 10`` (default) and ``-68 > -60 - 10`` holds true, the device will prioritize connecting to the 5 GHz AP.

    .. note::

        This parameter only takes effect when scanning results contain APs with the same SSID on both 2.4 GHz and 5 GHz bands. Its purpose is to avoid always connecting to a 2.4 GHz network with slightly stronger signal but poorer performance.

Station Basic Configuration
+++++++++++++++++++++++++++++++++++++

API :cpp:func:`esp_wifi_set_config()` can be used to configure the station. And the configuration will be stored in NVS. The table below describes the fields in detail.

.. list-table::
   :header-rows: 1
   :widths: 15 50

   * - Field
     - Description
   * - ssid
     - This is the SSID of the target AP, to which the station wants to connect.
   * - password
     - Password of the target AP.
   * - scan_method
     - For ``WIFI_FAST_SCAN`` scan, the scan ends when the first matched AP is found. For ``WIFI_ALL_CHANNEL_SCAN``, the scan finds all matched APs on all channels. The default scan is ``WIFI_FAST_SCAN``.
   * - bssid_set
     - If bssid_set is 0, the station connects to the AP whose SSID is the same as the field “ssid”, while the field “bssid” is ignored. In all other cases, the station connects to the AP whose SSID is the same as the “ssid” field, while its BSSID is the same the “bssid” field .
   * - bssid
     - This is valid only when bssid_set is 1; see field “bssid_set”.
   * - channel
     - If the channel is 0, the station scans the channel 1 ~ N to search for the target AP; otherwise, the station starts by scanning the channel whose value is the same as that of the “channel” field, and then scans the channel 1 ~ N but skip the specific channel to find the target AP. For example, if the channel is 3, the scan order will be 3, 1, 2, 4,..., N. If you do not know which channel the target AP is running on, set it to 0.
   * - sort_method
     - This field is only for ``WIFI_ALL_CHANNEL_SCAN``.

       If the sort_method is ``WIFI_CONNECT_AP_BY_SIGNAL``, all matched APs are sorted by signal, and the AP with the best signal will be connected firstly. For example, the station wants to connect an AP whose SSID is “apxx”. If the scan finds two APs whose SSID equals to “apxx”, and the first AP’s signal is -90 dBm while the second AP’s signal is -30 dBm, the station connects the second AP firstly, and it would not connect the first one unless it fails to connect the second one.

       If the sort_method is ``WIFI_CONNECT_AP_BY_SECURITY``, all matched APs are sorted by security. For example, the station wants to connect an AP whose SSID is “apxx”. If the scan finds two APs whose SSID is “apxx”, and the security of the first found AP is open while the second one is WPA2, the station connects to the second AP firstly, and it would not connect the first one unless it fails to connect the second one.
   * - threshold
     - The threshold is used to filter the found AP. If the RSSI or security mode is less than the configured threshold, the AP will be discarded.

       If the RSSI is set to 0, it means the default threshold and the default RSSI threshold are -127 dBm. If the authmode threshold is set to 0, it means the default threshold and the default authmode threshold are open.


.. attention::

    WEP/WPA security modes are deprecated in IEEE 802.11-2016 specifications and are recommended not to be used. These modes can be rejected using authmode threshold by setting threshold as WPA2 by threshold.authmode as ``WIFI_AUTH_WPA2_PSK``.

AP Basic Configuration
+++++++++++++++++++++++++++++++++++++

API :cpp:func:`esp_wifi_set_config()` can be used to configure the AP. And the configuration will be stored in NVS. The table below describes the fields in detail.

.. only:: esp32 or esp32s2 or esp32s3 or esp32c3 or esp32c5 or esp32c6

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Field
        - Description
      * - ssid
        - SSID of AP; if the ssid[0] is 0xFF and ssid[1] is 0xFF, the AP defaults the SSID to ``ESP_aabbcc``, where “aabbcc” is the last three bytes of the AP MAC.
      * - password
        - Password of AP; if the auth mode is ``WIFI_AUTH_OPEN``, this field will be ignored.
      * - ssid_len
        - Length of SSID; if ssid_len is 0, check the SSID until there is a termination character. If ssid_len > 32, change it to 32; otherwise, set the SSID length according to ssid_len.
      * - channel
        - Channel of AP; if the channel is out of range, the Wi-Fi driver will return error. So, please make sure the channel is within the required range. For more details, refer to `Wi-Fi Country Code`_.
      * - authmode
        - Auth mode of ESP AP; currently, ESP AP does not support AUTH_WEP. If the authmode is an invalid value, AP defaults the value to ``WIFI_AUTH_OPEN``.
      * - ssid_hidden
        - If ssid_hidden is 1, AP does not broadcast the SSID; otherwise, it does broadcast the SSID.
      * - max_connection
        - The max number of stations allowed to connect in, the default value is 10. ESP Wi-Fi supports up to {IDF_TARGET_MAX_CONN_STA_NUM} (``ESP_WIFI_MAX_CONN_NUM``) Wi-Fi connections. Please note that ESP AP and ESP-NOW share the same encryption hardware keys, so the max_connection parameter will be affected by the :ref:`CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM`. The total number of encryption hardware keys is 17, if :ref:`CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM` <= {IDF_TARGET_SUB_MAX_NUM_FROM_KEYS}, the max_connection can be set up to {IDF_TARGET_MAX_CONN_STA_NUM}, otherwise the max_connection can be set up to (17 - :ref:`CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM`).
      * - beacon_interval
        - Beacon interval; the value is 100 ~ 60000 ms, with default value being 100 ms. If the value is out of range, AP defaults it to 100 ms.


.. only:: esp32c2

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Field
        - Description
      * - ssid
        - SSID of AP; if the ssid[0] is 0xFF and ssid[1] is 0xFF, the AP defaults the SSID to ``ESP_aabbcc``, where “aabbcc” is the last three bytes of the AP MAC.
      * - password
        - Password of AP; if the auth mode is ``WIFI_AUTH_OPEN``, this field will be ignored.
      * - ssid_len
        - Length of SSID; if ssid_len is 0, check the SSID until there is a termination character. If ssid_len > 32, change it to 32; otherwise, set the SSID length according to ssid_len.
      * - channel
        - Channel of AP; if the channel is out of range, the Wi-Fi driver defaults to channel 1. So, please make sure the channel is within the required range. For more details, refer to `Wi-Fi Country Code`_.
      * - authmode
        - Auth mode of ESP AP; currently, ESP AP does not support AUTH_WEP. If the authmode is an invalid value, AP defaults the value to ``WIFI_AUTH_OPEN``.
      * - ssid_hidden
        - If ssid_hidden is 1, AP does not broadcast the SSID; otherwise, it does broadcast the SSID.
      * - max_connection
        - The max number of stations allowed to connect in, the default value is 2. ESP Wi-Fi supports up to {IDF_TARGET_MAX_CONN_STA_NUM} (``ESP_WIFI_MAX_CONN_NUM``) Wi-Fi connections. Please note that ESP AP and ESP-NOW share the same encryption hardware keys, so the max_connection parameter will be affected by the :ref:`CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM`. The total number of encryption hardware keys is {IDF_TARGET_MAX_CONN_STA_NUM}, the max_connection can be set up to ({IDF_TARGET_MAX_CONN_STA_NUM} - :ref:`CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM`).
      * - beacon_interval
        - Beacon interval; the value is 100 ~ 60000 ms, with default value being 100 ms. If the value is out of range, AP defaults it to 100 ms.


Wi-Fi Protocol Mode
+++++++++++++++++++++++++

Currently, the ESP-IDF supports the following protocol modes:

.. only:: esp32 or esp32s2 or esp32c3 or esp32s3

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Protocol Mode
        - Description
      * - 802.11b
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B)`` to set the station/AP to 802.11b-only mode.
      * - 802.11bg
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G)`` to set the station/AP to 802.11bg mode.
      * - 802.11g
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G)`` and esp_wifi_config_11b_rate(ifx, true) to set the station/AP to 802.11g mode.
      * - 802.11bgn
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)`` to set the station/ AP to BGN mode.
      * - 802.11gn
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)`` and esp_wifi_config_11b_rate(ifx, true) to set the station/AP to 802.11gn mode.
      * - 802.11 BGNLR
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR)`` to set the station/AP to BGN and the LR mode.
      * - 802.11 LR
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_LR)`` to set the station/AP only to the LR mode.

          **This mode is an Espressif-patented mode which can achieve a one-kilometer line of sight range. Please make sure both the station and the AP are connected to an ESP device.**

.. only:: esp32c6

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Protocol Mode
        - Description
      * - 802.11b
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B)`` to set the station/AP to 802.11b-only mode.
      * - 802.11bg
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G)`` to set the station/AP to 802.11bg mode.
      * - 802.11g
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G)`` and ``esp_wifi_config_11b_rate(ifx, true)`` to set the station/AP to 802.11g mode.
      * - 802.11bgn
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)`` to set the station/ AP to BGN mode.
      * - 802.11gn
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)`` and esp_wifi_config_11b_rate(ifx, true) to set the station/AP to 802.11gn mode.
      * - 802.11 BGNLR
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR)`` to set the station/AP to BGN and the LR mode.
      * - 802.11bgnax
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_11AX)`` to set the station/ AP to 802.11bgnax mode.
      * - 802.11 BGNAXLR
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_11AX|WIFI_PROTOCOL_LR)`` to set the station/ AP to 802.11bgnax and LR mode.
      * - 802.11 LR
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_LR)`` to set the station/AP only to the LR mode.

          **This mode is an Espressif-patented mode which can achieve a one-kilometer line of sight range. Please make sure both the station and the AP are connected to an ESP device.**

.. only:: esp32c5

    - **2.4 GHz band**: Supports 802.11b, 802.11bg, 802.11bgn, 802.11bgnax, and Espressif's proprietary LR mode.
    - **5 GHz band**: Supports 802.11a, 802.11an, 802.11anac, and 802.11anacax.

    {IDF_TARGET_NAME} supports configuring Wi-Fi protocol modes for the 2.4 GHz and 5 GHz bands separately. It is recommended to use :cpp:func:`esp_wifi_set_protocols()` for this purpose. The legacy API :cpp:func:`esp_wifi_set_protocol()` is also supported.

    **Recommended Usage**

    Use the new API :cpp:func:`esp_wifi_set_protocols()` to configure each band independently:

    .. code-block:: c

        // Set 2.4 GHz to use 802.11bgnax, and 5 GHz to use 802.11anacax
        wifi_protocols_t protocols = {
            .ghz_2g = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX,
            .ghz_5g = WIFI_PROTOCOL_11A | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AC | WIFI_PROTOCOL_11AX,
        };
        esp_wifi_set_protocols(WIFI_IF_STA, &protocols);

    **Legacy Usage**

    Use the legacy API :cpp:func:`esp_wifi_set_protocol()` to configure the protocol mode for 2.4 GHz band or 5 GHz band:

    .. code-block:: c

        // Set band mode to 2.4 GHz band
        esp_wifi_set_band_mode(WIFI_BAND_MODE_2G_ONLY);

        // Set protocol of station to 802.11bgnax
        uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX;
        esp_wifi_set_protocol(WIFI_IF_STA, protocol);

    .. note::

        - The new API `esp_wifi_set_protocols()` allows configuring both bands simultaneously and is recommended for use on {IDF_TARGET_NAME}.
        - The function ``esp_wifi_set_protocol()`` is suitable for single-band scenarios, such as when ``WIFI_BAND_MODE_2G_ONLY`` or ``WIFI_BAND_MODE_5G_ONLY`` is used. It only takes effect on the currently connected band. For example, if the interface is operating on the 5 GHz band, any configuration for the 2.4 GHz band will be ignored.
        - If the configuration includes unsupported protocol combinations, the function will return an error.
        - To enable Espressif's proprietary LR mode, make sure to include `WIFI_PROTOCOL_LR` in the 2.4 GHz protocol configuration.

.. only:: esp32c2

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Protocol Mode
        - Description
      * - 802.11b
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B)`` to set the station/AP to 802.11b-only mode.
      * - 802.11bg
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G)`` to set the station/AP to 802.11bg mode.
      * - 802.11g
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G) and esp_wifi_config_11b_rate(ifx, true)`` to set the station/AP to 802.11g mode.
      * - 802.11bgn
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)`` to set the station/ AP to BGN mode.
      * - 802.11gn
        - Call ``esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)`` and ``esp_wifi_config_11b_rate(ifx, true)`` to set the station/AP to 802.11gn mode.

Wi-Fi Bandwidth Mode
++++++++++++++++++++++

.. only:: esp32 or esp32s2 or esp32c3 or esp32s3

    {IDF_TARGET_NAME} currently supports 20 MHz and 40 MHz bandwidth modes, which are used in combination with protocol modes. Common combinations include:

    - **HT20**: 802.11n with 20 MHz bandwidth
    - **HT40**: 802.11n with 40 MHz bandwidth

    .. note::

        - The 40 MHz bandwidth mode is only supported in 802.11n mode.

    Applications can use the :cpp:func:`esp_wifi_set_bandwidth()` API to set the bandwidth mode of the current interface.

    Example:

    .. code-block:: c

        // Set STA interface protocol to 802.11bgn
        uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;
        esp_wifi_set_protocol(WIFI_IF_STA, protocol);

        // Set STA interface bandwidth to 40 MHz
        esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);

.. only:: esp32c6

    {IDF_TARGET_NAME} currently supports 20 MHz and 40 MHz bandwidth modes, which are used in combination with protocol modes. Common combinations include:

    - **HT20**: 802.11n with 20 MHz bandwidth
    - **HT40**: 802.11n with 40 MHz bandwidth
    - **HE20**: 802.11ax with 20 MHz bandwidth

    .. note::

        - The 40 MHz bandwidth mode is only supported in 802.11n mode.

    Applications can use the :cpp:func:`esp_wifi_set_bandwidth()` API to set the bandwidth mode of the current interface.

    Example:

    .. code-block:: c

        // Set protocol of station to 802.11bgn
        uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;
        esp_wifi_set_protocol(WIFI_IF_STA, protocol);

        // Set bandwidth of station to 40 MHz
        esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);

.. only:: esp32c5

    {IDF_TARGET_NAME} currently supports two bandwidth modes, 20 MHz and 40 MHz, which are used in combination with protocol modes. Common combinations include:

    - **HT20**: 802.11n/11an, 20 MHz bandwidth
    - **HT40**: 802.11n/11an, 40 MHz bandwidth
    - **HE20**: 802.11ax, 20 MHz bandwidth

    .. note::

        - The 40 MHz bandwidth mode is only supported under 802.11n (2.4 GHz) or 802.11an (5 GHz) modes.

    Applications can use the :cpp:func:`esp_wifi_set_bandwidths()` API to set independent bandwidths for 2.4 GHz and 5 GHz bands.

    Example:

    .. code-block:: c

        // Set 2.4 GHz to use 802.11bgnax, and 5 GHz to use 802.11an
        wifi_protocols_t protocols = {
            .ghz_2g = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX,
            .ghz_5g = WIFI_PROTOCOL_11A | WIFI_PROTOCOL_11N,
        };
        esp_wifi_set_protocols(WIFI_IF_STA, &protocols);

        // Set bandwidth to 20 MHz for 2.4 GHz band and 40 MHz for 5 GHz band
        wifi_bandwidths_t bw = {
            .ghz_2g = WIFI_BW_HT20,
            .ghz_5g = WIFI_BW_HT40
        };
        esp_wifi_set_bandwidths(WIFI_IF_STA, &bw);

    .. note::

        - When `.ghz_2g` is set to 0, only the 5 GHz bandwidth is updated, and the 2.4 GHz bandwidth remains unchanged.
        - When `.ghz_5g` is set to 0, only the 2.4 GHz bandwidth is updated, and the 5 GHz bandwidth remains unchanged.

    **Legacy Usage**

    Use the legacy API :cpp:func:`esp_wifi_set_bandwidth()` to configure the bandwidth of the 2.4 GHz or 5 GHz band:

    .. code-block:: c

          // Set band mode to 5 GHz band
          esp_wifi_set_band_mode(WIFI_BAND_MODE_5G_ONLY);

          // Set protocol of the station interface to 802.11an
          uint8_t protocol = WIFI_PROTOCOL_11A | WIFI_PROTOCOL_11N;
          esp_wifi_set_protocol(WIFI_IF_STA, protocol);

          // Set bandwidth of the station interface to 40 MHz
          esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);

    .. note::

        - The new API :cpp:func:`esp_wifi_set_bandwidths()` can configure both 2.4 GHz and 5 GHz bandwidths simultaneously, and is recommended for use on {IDF_TARGET_NAME}.
        - ``esp_wifi_set_bandwidth()`` is suitable for single-band scenarios, such as ``WIFI_BAND_MODE_2G_ONLY`` or ``WIFI_BAND_MODE_5G_ONLY`` modes. It only affects the currently connected band. For example, if the interface is on the 5 GHz band, any 2.4 GHz bandwidth settings will be ignored.
        - If the configured bandwidth is not supported on the current band, the function will return an error.

.. only:: esp32c2

    {IDF_TARGET_NAME} currently supports only 20 MHz bandwidth mode.

.. only:: esp32 or esp32s2 or esp32c3 or esp32s3 or esp32c5 or esp32c6

    Long Range (LR)
    +++++++++++++++++++++++++

    Long Range (LR) mode is an Espressif-patented Wi-Fi mode which can achieve a one-kilometer line of sight range. Compared to the traditional 802.11b mode, it has better reception sensitivity, stronger anti-interference ability, and longer transmission distance.

    LR Compatibility
    *************************

    Since LR is an Espressif-unique Wi-Fi mode operating on the 2.4 GHz band, only ESP32-series chips (excluding the ESP32-C2) support LR data transmission and reception. To ensure compatibility, the ESP32 devices should NOT use LR data rates when connected to non-LR-capable devices. This can be enforced by configuring the appropriate Wi-Fi mode:

    - If the negotiated mode supports LR, ESP32 devices may transmit data at LR rates.
    - Otherwise, they must default to traditional Wi-Fi data rates.

    The following table depicts the Wi-Fi mode negotiation on 2.4 GHz band:

    .. only:: esp32 or esp32s2 or esp32c3 or esp32s3

        +-------+-----+----+---+-------+------+-----+----+
        |AP\STA | BGN | BG | B | BGNLR | BGLR | BLR | LR |
        +=======+=====+====+===+=======+======+=====+====+
        | BGN   | BGN | BG | B | BGN   | BG   | B   | -  |
        +-------+-----+----+---+-------+------+-----+----+
        | BG    | BG  | BG | B | BG    | BG   | B   | -  |
        +-------+-----+----+---+-------+------+-----+----+
        | B     | B   | B  | B | B     | B    | B   | -  |
        +-------+-----+----+---+-------+------+-----+----+
        | BGNLR | -   | -  | - | BGNLR | BGLR | BLR | LR |
        +-------+-----+----+---+-------+------+-----+----+
        | BGLR  | -   | -  | - | BGLR  | BGLR | BLR | LR |
        +-------+-----+----+---+-------+------+-----+----+
        | BLR   | -   | -  | - | BLR   | BLR  | BLR | LR |
        +-------+-----+----+---+-------+------+-----+----+
        | LR    | -   | -  | - | LR    | LR   | LR  | LR |
        +-------+-----+----+---+-------+------+-----+----+

    .. only:: esp32c5 or esp32c6

        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | AP\STA  | BGNAX | BGN | BG | B | BGNAXLR | BGNLR | BGLR | BLR | LR |
        +=========+=======+=====+====+===+=========+=======+======+=====+====+
        | BGNAX   | BGAX  | BGN | BG | B | BGAX    | BGN   | BG   | B   | -  |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | BGN     | BGN   | BGN | BG | B | BGN     | BGN   | BG   | B   | -  |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | BG      | BG    | BG  | BG | B | BG      | BG    | BG   | B   | -  |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | B       | B     | B   | B  | B | B       | B     | B    | B   | -  |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | BGNAXLR | -     | -   | -  | - | BGAXLR  | BGNLR | BGLR | BLR | LR |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | BGNLR   | -     | -   | -  | - | BGNLR   | BGNLR | BGLR | BLR | LR |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | BGLR    | -     | -   | -  | - | BGLR    | BGLR  | BGLR | BLR | LR |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | BLR     | -     | -   | -  | - | BLR     | BLR   | BLR  | BLR | LR |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+
        | LR      | -     | -   | -  | - | LR      | LR    | LR   | LR  | LR |
        +---------+-------+-----+----+---+---------+-------+------+-----+----+

    In the above table, the row is the Wi-Fi mode of AP and the column is the Wi-Fi mode of station. The "-" indicates Wi-Fi mode of the AP and station are not compatible.

    According to the table, the following conclusions can be drawn:

    - For LR-enabled AP of {IDF_TARGET_NAME}, it is incompatible with traditional 802.11 mode, because the beacon is sent in LR mode.
    - For LR-enabled station of {IDF_TARGET_NAME} whose mode is NOT LR-only mode, it is compatible with traditional 802.11 mode.
    - If both station and AP are ESP32 series chips devices (except ESP32-C2) and both of them have enabled LR mode, the negotiated mode supports LR.

    If the negotiated Wi-Fi mode supports both traditional 802.11 mode and LR mode, it is the Wi-Fi driver's responsibility to automatically select the best data rate in different Wi-Fi modes and the application can ignore it.

    LR Impacts to Traditional Wi-Fi Device
    ***************************************

    The data transmission in LR rate has no impacts on the traditional Wi-Fi device because:

    - The CCA and backoff process in LR mode are consistent with 802.11 specification.
    - The traditional Wi-Fi device can detect the LR signal via CCA and do backoff.

    In other words, the transmission impact in LR mode is similar to that in 802.11b mode.

    LR Transmission Distance
    *************************

    The reception sensitivity gain of LR is about 4 dB larger than that of the traditional 802.11b mode. Theoretically, the transmission distance is about 2 to 2.5 times the distance of 11B.

    LR Throughput
    *************************

    The LR rate has very limited throughput, because the raw PHY data rate LR is 1/2 Mbps and 1/4 Mbps.

    When to Use LR
    *************************

    The general conditions for using LR are:

    - Both the AP and station are Espressif devices.
    - Long distance Wi-Fi connection and data transmission is required.
    - Data throughput requirements are very small, such as remote device control.

.. only:: esp32c5

    Dynamic Frequency Selection (DFS)
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++

    In the 5 GHz Wi-Fi band, certain channels (e.g., channels 52–144) share spectrum with critical systems such as weather radars. To avoid interference with these systems, Wi-Fi devices must perform specific detection and channel switching mechanisms before operating on these channels—this is known as **Dynamic Frequency Selection (DFS)**.

    Enabling DFS allows devices to access more 5 GHz channels, thereby increasing network capacity and reducing interference. This is especially beneficial in high-density deployments or applications that require high bandwidth. The range of DFS channels may vary by country or region; see :component_file:`esp_wifi/regulatory/esp_wifi_regulatory.txt` for details.

    {IDF_TARGET_NAME} supports DFS channels in the 5 GHz band, but only supports **passive radar detection**.

    .. list-table::
      :header-rows: 1
      :widths: 20 60

      * - Type
        - Description
      * - Passive Radar Detection Supported
        - During scanning, the device can listen to DFS channels, detect access points (APs) operating on DFS channels, and connect to them. If the AP detects radar signals and switches to another channel using Channel Switch Announcement (CSA), {IDF_TARGET_NAME} will follow the AP to the new channel.
      * - Active Radar Detection Not Supported
        - {IDF_TARGET_NAME} **does not support** active radar detection and therefore cannot operate as a DFS AP in SoftAP mode.

    .. note::
      - In STA mode, {IDF_TARGET_NAME} can connect to APs operating on DFS channels, provided those channels are detected during scanning.
      - In SoftAP mode, {IDF_TARGET_NAME} is not allowed to operate on DFS channels to comply with regulatory requirements.
      - In STA+SoftAP coexistence mode:
        1. If the STA connects to an AP on a DFS channel, the SoftAP is allowed to switch to the same DFS channel using CSA (Channel Switch Announcement).
        2. When the STA disconnects, the SoftAP will switch back to a non-DFS channel via CSA to remain compliant with regulations.

Wi-Fi Country Code
+++++++++++++++++++++++++

.. only:: esp32 or esp32c2 or esp32s2 or esp32c3 or esp32s3 or esp32c6

    Call :cpp:func:`esp_wifi_set_country()` to set the country info. The table below describes the fields in detail. Please consult local 2.4 GHz RF operating regulations before configuring these fields.

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Field
        - Description
      * - cc[3]
        - Country code string. This attribute identifies the country or noncountry entity in which the station/AP is operating. If it is a country, the first two octets of this string is the two-character country info as described in the document ISO/IEC3166-1. The third octet is one of the following:

          - an ASCII space character, which means the regulations under which the station/AP is operating encompass all environments for the current frequency band in the country.
          - an ASCII ‘O’ character, which means the regulations under which the station/AP is operating are for an outdoor environment only.
          - an ASCII ‘I’ character, which means the regulations under which the station/AP is operating are for an indoor environment only.
          - an ASCII ‘X’ character, which means the station/AP is operating under a noncountry entity. The first two octets of the noncountry entity is two ASCII ‘XX’ characters.
          - the binary representation of the Operating Class table number currently in use. Refer to Annex E of IEEE Std 802.11-2020.

      * - schan
        - Start channel. It is the minimum channel number of the regulations under which the station/AP can operate.
      * - nchan
        - Total number of channels as per the regulations. For example, if the schan=1, nchan=13, then the station/AP can send data from channel 1 to 13.
      * - policy
        - Country policy. This field controls which country info will be used if the configured country info is in conflict with the connected AP’s. For more details on related policies, see the following section.


    The default country info is::

        wifi_country_t config = {
            .cc = "01",
            .schan = 1,
            .nchan = 11,
          .policy = WIFI_COUNTRY_POLICY_AUTO,
        };

    If the Wi-Fi Mode is station/AP coexist mode, they share the same configured country info. Sometimes, the country info of AP, to which the station is connected, is different from the country info of configured. For example, the configured station has country info::

        wifi_country_t config = {
            .cc = "JP",
            .schan = 1,
            .nchan = 14,
            .policy = WIFI_COUNTRY_POLICY_AUTO,
        };

    but the connected AP has country info::

        wifi_country_t config = {
            .cc = "CN",
            .schan = 1,
            .nchan = 13,
        ;

    then country info of connected AP's is used.

    The following table depicts which country info is used in different Wi-Fi modes and different country policies, and it also describes the impact on active scan.

    .. list-table::
      :header-rows: 1
      :widths: 15 15 35

      * - Wi-Fi Mode
        - Policy
        - Description
      * - Station
        - WIFI_COUNTRY_POLICY_AUTO
        - If the connected AP has country IE in its beacon, the country info equals to the country info in beacon. Otherwise, use the default country info.

          For scan:

            Use active scan from 1 to 11 and use passive scan from 12 to 14.

          Always keep in mind that if an AP with hidden SSID and station is set to a passive scan channel, the passive scan will not find it. In other words, if the application hopes to find the AP with hidden SSID in every channel, the policy of country info should be configured to WIFI_COUNTRY_POLICY_MANUAL.

      * - Station
        - WIFI_COUNTRY_POLICY_MANUAL
        - Always use the configured country info.

          For scan:

            Use active scan from schan to schan+nchan-1.

      * - AP
        - WIFI_COUNTRY_POLICY_AUTO
        - Always use the configured country info.

      * - AP
        - WIFI_COUNTRY_POLICY_MANUAL
        - Always use the configured country info.

      * - Station/AP-coexistence
        - WIFI_COUNTRY_POLICY_AUTO
        - Station: Same as station mode with policy WIFI_COUNTRY_POLICY_AUTO.
          AP: If the station does not connect to any external AP, the AP uses the configured country info. If the station connects to an external AP, the AP has the same country info as the station.

      * - Station/AP-coexistence
        - WIFI_COUNTRY_POLICY_MANUAL
        - Station: Same as station mode with policy WIFI_COUNTRY_POLICY_MANUAL.
          AP: Same as AP mode with policy WIFI_COUNTRY_POLICY_MANUAL.

.. only:: esp32c5

    The :cpp:func:`esp_wifi_set_country()` function is used to set the country/region information. The table below details the meaning of each field. Before configuring these fields, please refer to local 2.4 GHz and 5 GHz RF regulations.

    .. list-table::
      :header-rows: 1
      :widths: 15 55

      * - Field
        - Description
      * - cc[3]
        - Country/region code string identifying the country or region of the station/AP, or a non-country entity. If it is a country/region, the first two bytes comply with the ISO/IEC 3166-1 two-letter country code standard. The third byte has the following meanings:

          - Space character (ASCII): The country/region allows usage of the respective frequency band in all environments.
          - Character `'O'`: Usage allowed only in outdoor environments.
          - Character `'I'`: Usage allowed only in indoor environments.
          - Character `'X'`: Non-country entity, in which case the first two characters should be `'XX'`.
          - Binary operational class number, referring to IEEE Std 802.11-2020 Appendix E.

      * - schan
        - Start channel number, indicating the minimum channel allowed in the 2.4 GHz band for the configured country/region.
      * - nchan
        - Number of channels. Defines the total number of allowed channels in the 2.4 GHz band. For example, if `schan = 1` and `nchan = 13`, then channels 1 through 13 are allowed.
      * - policy
        - Country/region policy. When the configured country/region conflicts with that of the connected AP, this field determines which information to use. Details are explained below.
      * - wifi_5g_channel_mask
        - Bitmask indicating allowed 5 GHz channels for the station/AP. The mapping between channel numbers and bits can be found in :cpp:enum:`wifi_5g_channel_bit_t`.

    A default configuration example::

        wifi_country_t config = {
            .cc = "01",
            .schan = 1,
            .nchan = 11,
            .wifi_5g_channel_mask = 0xfe,
            .policy = WIFI_COUNTRY_POLICY_AUTO,
        };

    When Wi-Fi is in station/AP coexistence mode, both use the same country/region information. Sometimes, the connected AP's country/region information may differ from the station's preset. For example:

    Station configuration::

        wifi_country_t config = {
            .cc = "JP",
            .schan = 1,
            .nchan = 14,
            .wifi_5g_channel_mask = 0xfe,
            .policy = WIFI_COUNTRY_POLICY_AUTO,
        };

    Connected AP configuration::

        wifi_country_t config = {
            .cc = "CN",
            .schan = 1,
            .nchan = 13,
        };

    In this case, the connected AP's country/region information will be used.

    The following table explains which country/region information is used under different Wi-Fi modes and policies, as well as differences in scanning behavior:

    .. list-table::
      :header-rows: 1
      :widths: 15 15 35

      * - Wi-Fi Mode
        - Policy
        - Description
      * - Station mode
        - WIFI_COUNTRY_POLICY_AUTO
        - If the connected AP's beacon contains country/region information IE, that information is used; otherwise, the default configuration is used.

          Scanning behavior:

            - 2.4 GHz band: active scanning on channels 1–11, passive scanning on channels 12–14;
            - 5 GHz band: active scanning on non-DFS channels, passive scanning on DFS channels.

          Note: If an AP with a hidden SSID is on a passive scan channel, scanning may not discover that AP. To discover hidden SSIDs on all channels, use `WIFI_COUNTRY_POLICY_MANUAL`.

      * - Station mode
        - WIFI_COUNTRY_POLICY_MANUAL
        - Always use the configured country/region information.

          Scanning behavior:

            - 2.4 GHz band: scan channels from `schan` to `schan + nchan - 1`;
            - 5 GHz band: scan channels supported as indicated by `wifi_5g_channel_mask`.

      * - AP mode
        - WIFI_COUNTRY_POLICY_AUTO
        - Always use the configured country/region information.
      * - AP mode
        - WIFI_COUNTRY_POLICY_MANUAL
        - Always use the configured country/region information.
      * - Station/AP coexistence mode
        - WIFI_COUNTRY_POLICY_AUTO
        - Same behavior as station mode with `WIFI_COUNTRY_POLICY_AUTO`.

          If the station is not connected to any AP, the AP uses the configured country/region information;
          if the station is connected to an external AP, the AP uses the country/region information obtained by the station.

      * - Station/AP coexistence mode
        - WIFI_COUNTRY_POLICY_MANUAL
        - Same behavior as station mode with `WIFI_COUNTRY_POLICY_MANUAL`. The AP always uses the configured country/region information.

Home Channel
*************************

In AP mode, the home channel is defined as the AP channel. In station mode, home channel is defined as the channel of AP which the station is connected to. In station/AP-coexistence mode, the home channel of AP and station must be the same, and if they are different, the station's home channel is always in priority. For example, assume that the AP is on channel 6, and the station connects to an AP whose channel is 9. Since the station's home channel has higher priority, the AP needs to switch its channel from 6 to 9 to make sure that it has the same home channel as the station. While switching channel, the {IDF_TARGET_NAME} in AP mode will notify the connected stations about the channel migration using a Channel Switch Announcement (CSA). Station that supports channel switching will transit without disconnecting and reconnecting to the AP.


Wi-Fi Vendor IE Configuration
+++++++++++++++++++++++++++++++++++

By default, all Wi-Fi management frames are processed by the Wi-Fi driver, and the application can ignore them. However, some applications may have to handle the beacon, probe request, probe response, and other management frames. For example, if you insert some vendor-specific IE into the management frames, it is only the management frames which contain this vendor-specific IE that will be processed. In {IDF_TARGET_NAME}, :cpp:func:`esp_wifi_set_vendor_ie()` and :cpp:func:`esp_wifi_set_vendor_ie_cb()` are responsible for this kind of tasks.

Wi-Fi Easy Connect™ (DPP)
--------------------------

Wi-Fi Easy Connect\ :sup:`TM` (or Device Provisioning Protocol) is a secure and standardized provisioning protocol for configuring Wi-Fi devices. More information can be found in :doc:`esp_dpp <../api-reference/network/esp_dpp>`.

WPA2-Enterprise
---------------

WPA2-Enterprise is the secure authentication mechanism for enterprise wireless networks. It uses RADIUS server for authentication of network users before connecting to the Access Point. The authentication process is based on 802.1X policy and comes with different Extended Authentication Protocol (EAP) methods such as TLS, TTLS, and PEAP. RADIUS server authenticates the users based on their credentials (username and password), digital certificates, or both. When {IDF_TARGET_NAME} in station mode tries to connect an AP in enterprise mode, it sends authentication request to AP which is sent to RADIUS server by AP for authenticating the station. Based on different EAP methods, the parameters can be set in configuration which can be opened using ``idf.py menuconfig``. WPA2_Enterprise is supported by {IDF_TARGET_NAME} only in station mode.

For establishing a secure connection, AP and station negotiate and agree on the best possible cipher suite to be used. {IDF_TARGET_NAME} supports 802.1X/EAP (WPA) method of AKM and Advanced encryption standard with Counter Mode Cipher Block Chaining Message Authentication protocol (AES-CCM) cipher suite. It also supports the cipher suites supported by mbedtls if `USE_MBEDTLS_CRYPTO` flag is set.

{IDF_TARGET_NAME} currently supports the following EAP methods:
  - EAP-TLS: This is a certificate-based method and only requires SSID and EAP-IDF.
  - PEAP: This is a Protected EAP method. Username and Password are mandatory.
  - EAP-TTLS: This is a credential-based method. Only server authentication is mandatory while user authentication is optional. Username and Password are mandatory. It supports different Phase2 methods, such as:
     - PAP: Password Authentication Protocol.
     - CHAP: Challenge Handshake Authentication Protocol.
     - MSCHAP and MSCHAP-V2.
  - EAP-FAST: This is an authentication method based on Protected Access Credentials (PAC) which also uses identity and password. Currently, USE_MBEDTLS_CRYPTO flag should be disabled to use this feature.

Detailed information on creating certificates and how to run wpa2_enterprise example on {IDF_TARGET_NAME} can be found in :example:`wifi/wifi_enterprise`.

.. only:: SOC_WIFI_NAN_SUPPORT

    Wi-Fi Aware\ :sup:`TM` (NAN)
    ----------------------------

    Wi-Fi Aware\ :sup:`TM` or NAN (Neighbor Awareness Networking) is a protocol that allows Wi-Fi devices to discover services in their proximity. NAN uses direct device-to-device communication and does not require any Internet or AP connection.

    Multiple NAN devices in the vicinity will form a NAN cluster which allows them to communicate with each other. NAN devices in a cluster synchronise their clocks and listen to each other periodically on Channel 6. Devices can advertise (Publish) or seek for (Subscribe) services within their NAN Cluster using Service Discovery protocols. Matching of services is done by service name and optionally matching filters. Once a Subscriber gets a match with a Publisher, it can either send a message (Follow-up) or establish a datapath (NDP) with the Publisher. After NDP is setup both devices will obtain an IPv6 address and can use it for communication.

    Please note that NAN Datapath security is not supported i.e., the data packets will go out unencrypted. NAN uses a separate interface for Discovery and Datapath, which is other than that used for STA and AP. NAN operates in standalone mode, which means co-existence with STA or AP interface is not supported.

    Refer to ESP-IDF examples :idf_file:`examples/wifi/wifi_aware/nan_publisher/README.md` and :idf_file:`examples/wifi/wifi_aware/nan_subscriber/README.md` to setup a NAN Publisher and Subscriber.

Wireless Network Management
----------------------------

Wireless Network Management allows client devices to exchange information about the network topology, including information related to RF environment. This makes each client network-aware, facilitating overall improvement in the performance of the wireless network. It is part of 802.11v specification. It also enables the client to support Network assisted Roaming.
- Network assisted Roaming: Enables WLAN to send messages to associated clients, resulting clients to associate with APs with better link metrics. This is useful for both load balancing and in directing poorly connected clients.

Current implementation of 802.11v includes support for BSS transition management frames.

Radio Resource Measurement
---------------------------

Radio Resource Measurement (802.11k) is intended to improve the way traffic is distributed within a network. In a WLAN, each device normally connects to the access point (AP) that provides the strongest signal. Depending on the number and geographic locations of the subscribers, this arrangement can sometimes lead to excessive demand on one AP and underutilization of others, resulting in degradation of overall network performance. In a network conforming to 802.11k, if the AP having the strongest signal is loaded to its full capacity, a wireless device can be moved to one of the underutilized APs. Even though the signal may be weaker, the overall throughput is greater because more efficient use is made of the network resources.

Current implementation of 802.11k includes support for beacon measurement report, link measurement report, and neighbor request.

Refer ESP-IDF example :idf_file:`examples/wifi/roaming/README.md` to set up and use these APIs. Example code only demonstrates how these APIs can be used, and the application should define its own algorithm and cases as required.

Fast BSS Transition
---------------------------
Fast BSS transition (802.11R FT), is a standard to permit continuous connectivity aboard wireless devices in motion, with fast and secure client transitions from one Basic Service Set (abbreviated BSS, and also known as a base station or more colloquially, an access point) to another performed in a nearly seamless manner **avoiding 802.1i 4 way handshake** . 802.11R specifies transitions between access points by redefining the security key negotiation protocol, allowing both the negotiation and requests for wireless resources to occur in parallel. The key derived from the server to be cached in the wireless network, so that a reasonable number of future connections can be based on the cached key, avoiding the 802.1X process


{IDF_TARGET_NAME} station supports FT for WPA2-PSK networks. Do note that {IDF_TARGET_NAME} station only support FT over the air protocol only.

A config option :ref:`CONFIG_ESP_WIFI_11R_SUPPORT` and configuration parameter :cpp:type:`ft_enabled` in :cpp:type:`wifi_sta_config_t` is provided to enable 802.11R support for station. Refer ESP-IDF example :idf_file:`examples/wifi/roaming/README.md` for further details.

.. only:: SOC_WIFI_FTM_SUPPORT

    Wi-Fi Location
    -------------------------------

    Wi-Fi Location will improve the accuracy of a device's location data beyond the Access Point, which will enable creation of new and feature-rich applications and services such as geo-fencing, network management, and navigation. One of the protocols used to determine the device location with respect to the Access Point is Fine Timing Measurement which calculates Time-of-Flight of a Wi-Fi frame.

    Fine Timing Measurement (FTM)
    +++++++++++++++++++++++++++++

    FTM is used to measure Wi-Fi Round Trip Time (Wi-Fi RTT) which is the time a Wi-Fi signal takes to travel from a device to another device and back again. Using Wi-Fi RTT, the distance between the devices can be calculated with a simple formula of `RTT * c / 2`, where c is the speed of light.

    FTM uses timestamps given by Wi-Fi interface hardware at the time of arrival or departure of frames exchanged between a pair of devices. One entity called FTM Initiator (mostly a station device) discovers the FTM Responder (can be a station or an Access Point) and negotiates to start an FTM procedure. The procedure uses multiple Action frames sent in bursts and its ACK's to gather the timestamps data. FTM Initiator gathers the data in the end to calculate an average Round-Trip-Time.

    {IDF_TARGET_NAME} supports FTM in below configuration:

    - {IDF_TARGET_NAME} as FTM Initiator in station mode.
    - {IDF_TARGET_NAME} as FTM Responder in AP mode.

.. only:: esp32c6

   {IDF_TARGET_NAME} ECO1 and older versions do not support FTM Initiator mode.

.. attention::

    Distance measurement using RTT is not accurate, and factors such as RF interference, multi-path travel, antenna orientation, and lack of calibration increase these inaccuracies. For better results, it is suggested to perform FTM between two ESP32 chip series devices as station and AP.

    Refer to ESP-IDF example :idf_file:`examples/wifi/ftm/README.md` for steps on how to set up and perform FTM.

{IDF_TARGET_NAME} Wi-Fi Power-saving Mode
-----------------------------------------

This subsection will briefly introduce the concepts and usage related to Wi-Fi Power Saving Mode, for a more detailed introduction please refer to the :doc:`Low Power Mode User Guide <../api-guides/low-power-mode/index>`

Station Sleep
++++++++++++++++++++++

Currently, {IDF_TARGET_NAME} Wi-Fi supports the Modem-sleep mode which refers to the legacy power-saving mode in the IEEE 802.11 protocol. Modem-sleep mode works in station-only mode and the station must connect to the AP first. If the Modem-sleep mode is enabled, station will switch between active and sleep state periodically. In sleep state, RF, PHY and BB are turned off in order to reduce power consumption. Station can keep connection with AP in modem-sleep mode.

Modem-sleep mode includes minimum and maximum power-saving modes. In minimum power-saving mode, station wakes up every DTIM to receive beacon. Broadcast data will not be lost because it is transmitted after DTIM. However, it cannot save much more power if DTIM is short for DTIM is determined by AP.

In maximum power-saving mode, station wakes up in every listen interval to receive beacon. This listen interval can be set to be longer than the AP DTIM period. Broadcast data may be lost because station may be in sleep state at DTIM time. If listen interval is longer, more power is saved, but broadcast data is more easy to lose. Listen interval can be configured by calling API :cpp:func:`esp_wifi_set_config()` before connecting to AP.

Call ``esp_wifi_set_ps(WIFI_PS_MIN_MODEM)`` to enable Modem-sleep minimum power-saving mode or ``esp_wifi_set_ps(WIFI_PS_MAX_MODEM)`` to enable Modem-sleep maximum power-saving mode after calling :cpp:func:`esp_wifi_init()`. When station connects to AP, Modem-sleep will start. When station disconnects from AP, Modem-sleep will stop.

Call ``esp_wifi_set_ps(WIFI_PS_NONE)`` to disable Modem-sleep mode entirely. Disabling it increases power consumption, but minimizes the delay in receiving Wi-Fi data in real time. When Modem-sleep mode is enabled, the delay in receiving Wi-Fi data may be the same as the DTIM cycle (minimum power-saving mode) or the listening interval (maximum power-saving mode).

.. only:: SOC_SUPPORT_COEXISTENCE

    Note that in coexist mode, Wi-Fi will remain active only during Wi-Fi time slice, and sleep during non Wi-Fi time slice even if ``esp_wifi_set_ps(WIFI_PS_NONE)`` is called. Please refer to :ref:`coexist policy <coexist_policy>`.

The default Modem-sleep mode is WIFI_PS_MIN_MODEM.

AP Sleep
+++++++++++++++++++++++++++++++

Currently, {IDF_TARGET_NAME} AP does not support all of the power-saving feature defined in Wi-Fi specification. To be specific, the AP only caches unicast data for the stations connect to this AP, but does not cache the multicast data for the stations. If stations connected to the {IDF_TARGET_NAME} AP are power-saving enabled, they may experience multicast packet loss.

In the future, all power-saving features will be supported on {IDF_TARGET_NAME} AP.

Disconnected State Sleep
+++++++++++++++++++++++++++++++

Disconnected state is the duration without Wi-Fi connection between :cpp:func:`esp_wifi_start` to :cpp:func:`esp_wifi_stop`.

Currently, {IDF_TARGET_NAME} Wi-Fi supports sleep mode in disconnected state if running at station mode. This feature could be configured by Menuconfig choice :ref:`CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE`.

If :ref:`CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE` is enabled, RF, PHY and BB would be turned off in disconnected state when IDLE. The current would be same with current at modem-sleep.

The choice :ref:`CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE` would be selected by default, while it would be selected forcefully in Menuconfig at coexistence mode.

.. _connectionless-module-power-save:

Connectionless Modules Power-saving
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Connectionless modules are those Wi-Fi modules not relying on Wi-Fi connection, e.g ESP-NOW, DPP, FTM. These modules start from :cpp:func:`esp_wifi_start`, working until :cpp:func:`esp_wifi_stop`.

Currently, if ESP-NOW works at station mode, its supported to sleep at both connected state and disconnected state.

Connectionless Modules TX
*******************************

For each connectionless module, its supported to TX at any sleeping time without any extra configuration.

Meanwhile, :cpp:func:`esp_wifi_80211_tx` is supported at sleep as well.

Connectionless Modules RX
*******************************

For each connectionless module, two parameters shall be configured to RX at sleep, which are `Window` and `Interval`.

At the start of `Interval` time, RF, PHY, BB would be turned on and kept for `Window` time. Connectionless Module could RX in the duration.

**Interval**

 - There is only one `Interval`. Its configured by :cpp:func:`esp_wifi_set_connectionless_interval`. The unit is milliseconds.

 - The default value of `Interval` is `ESP_WIFI_CONNECTIONLESS_INTERVAL_DEFAULT_MODE`.

 - Event `WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START`_ would be posted at the start of `Interval`. Since `Window` also starts at that moment, its recommended to TX in that event.

 - At connected state, the start of `Interval` would be aligned with TBTT. To improve the packet reception success rate in connectionless modules, the sender and receiver can be connected to the same AP, and packets can be transmitted within the event `WIFI_EVENT_CONNECTIONLESS_MODULE_WAKE_INTERVAL_START`_. This synchronization helps align the connectionless modules transmission window.

 .. only:: esp32

    On the ESP32, TBTT timing is affected by DFS(Dynamic Frequency Scaling). To synchronize the connectionless modules transmission window using TBTT on the ESP32, DFS must be disabled.

**Window**

 - Each connectionless module has its own `Window` after start. Connectionless Modules Power-saving would work with the max one among them.

 - `Window` is configured by :cpp:func:`module_name_set_wake_window`. The unit is milliseconds.

 - The default value of `Window` is the maximum.

.. table:: RF, PHY and BB usage under different circumstances

    +----------------------+-----------------------------------------------------+---------------------------------------------------------------------------+
    |                      | Interval                                                                                                                        |
    +                      +-----------------------------------------------------+---------------------------------------------------------------------------+
    |                      | ``ESP_WIFI_CONNECTIONLESS_INTERVAL_DEFAULT_MODE``   | 1 - maximum                                                               |
    +--------+-------------+-----------------------------------------------------+---------------------------------------------------------------------------+
    | Window | 0           | not used                                                                                                                        |
    +        +-------------+-----------------------------------------------------+---------------------------------------------------------------------------+
    |        | 1 - maximum | default mode                                        | used periodically (Window < Interval) / used all time (Window ≥ Interval) |
    +--------+-------------+-----------------------------------------------------+---------------------------------------------------------------------------+

Default Mode
*******************************

If `Interval` is ``ESP_WIFI_CONNECTIONLESS_INTERVAL_DEFAULT_MODE`` with non-zero `Window`, Connectionless Modules Power-saving would work in default mode.

In default mode, RF, PHY, BB would be kept on if no coexistence with non-Wi-Fi protocol.

With coexistence, RF, PHY, BB resources are allocated by coexistence module to Wi-Fi connectionless module and non-Wi-Fi module, using time-division method. In default mode, Wi-Fi connectionless module is allowed to use RF, BB, PHY periodically under a stable performance.

Its recommended to configure Connectionless Modules Power-saving to default mode if there is Wi-Fi connectionless module coexists with non-Wi-Fi module.

{IDF_TARGET_NAME} Wi-Fi Throughput
-----------------------------------

The table below shows the best throughput results gained in Espressif's lab and in a shielded box.

.. only:: esp32

    .. list-table::
       :header-rows: 1
       :widths: 10 10 10 10 25

       * - Type/Throughput
         - Air In Lab
         - Shield-box
         - Test Tool
         - IDF Version (commit ID)
       * - Raw 802.11 Packet RX
         - N/A
         - **130 MBit/s**
         - Internal tool
         - NA
       * - Raw 802.11 Packet TX
         - N/A
         - **130 MBit/s**
         - Internal tool
         - NA
       * - UDP RX
         - 30 MBit/s
         - 85 MBit/s
         - iperf example
         - 15575346
       * - UDP TX
         - 30 MBit/s
         - 75 MBit/s
         - iperf example
         - 15575346
       * - TCP RX
         - 20 MBit/s
         - 65 MBit/s
         - iperf example
         - 15575346
       * - TCP TX
         - 20 MBit/s
         - 75 MBit/s
         - iperf example
         - 15575346

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32`.

.. only:: esp32s2

    .. list-table::
       :header-rows: 1
       :widths: 10 10 10 10 25

       * - Type/Throughput
         - Air In Lab
         - Shield-box
         - Test Tool
         - IDF Version (commit ID)
       * - Raw 802.11 Packet RX
         - N/A
         - **130 MBit/s**
         - Internal tool
         - NA
       * - Raw 802.11 Packet TX
         - N/A
         - **130 MBit/s**
         - Internal tool
         - NA
       * - UDP RX
         - 30 MBit/s
         - 70 MBit/s
         - iperf example
         - 15575346
       * - UDP TX
         - 30 MBit/s
         - 50 MBit/s
         - iperf example
         - 15575346
       * - TCP RX
         - 20 MBit/s
         - 32 MBit/s
         - iperf example
         - 15575346
       * - TCP TX
         - 20 MBit/s
         - 37 MBit/s
         - iperf example
         - 15575346

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32s2`.

.. only:: esp32c3

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15 20

        * - Type/Throughput
          - Air In Lab
          - Shield-box
          - Test Tool
          - IDF Version (commit ID)
        * - Raw 802.11 Packet RX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - Raw 802.11 Packet TX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - UDP RX
          - 30 MBit/s
          - 50 MBit/s
          - iperf example
          - 15575346
        * - UDP TX
          - 30 MBit/s
          - 40 MBit/s
          - iperf example
          - 15575346
        * - TCP RX
          - 20 MBit/s
          - 35 MBit/s
          - iperf example
          - 15575346
        * - TCP TX
          - 20 MBit/s
          - 37 MBit/s
          - iperf example
          - 15575346

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32c3`.

.. only:: esp32c5

    - 2.4 GHz band

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15 20

        * - Type/Throughput
          - Air In Lab
          - Shield-box
          - Test Tool
          - IDF Version (commit ID)
        * - Raw 802.11 Packet RX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - Raw 802.11 Packet TX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - UDP RX
          - 30 MBit/s
          - 68 MBit/s
          - iperf example
          - 7ff0a07d
        * - UDP TX
          - 30 MBit/s
          - 63 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP RX
          - 20 MBit/s
          - 59 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP TX
          - 20 MBit/s
          - 49 MBit/s
          - iperf example
          - 7ff0a07d

    - 5 GHz band

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15 20

        * - Type/Throughput
          - Air In Lab
          - Shield-box
          - Test Tool
          - IDF Version (commit ID)
        * - Raw 802.11 Packet RX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - Raw 802.11 Packet TX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - UDP RX
          - 30 MBit/s
          - 71 MBit/s
          - iperf example
          - 7ff0a07d
        * - UDP TX
          - 30 MBit/s
          - 64 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP RX
          - 20 MBit/s
          - 61 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP TX
          - 20 MBit/s
          - 50 MBit/s
          - iperf example
          - 7ff0a07d

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32c5`.

.. only:: esp32c6

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15 20

        * - Type/Throughput
          - Air In Lab
          - Shield-box
          - Test Tool
          - IDF Version (commit ID)
        * - Raw 802.11 Packet RX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - Raw 802.11 Packet TX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - UDP RX
          - 30 MBit/s
          - 63 MBit/s
          - iperf example
          - 7ff0a07d
        * - UDP TX
          - 30 MBit/s
          - 51 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP RX
          - 20 MBit/s
          - 46 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP TX
          - 20 MBit/s
          - 43 MBit/s
          - iperf example
          - 7ff0a07d

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32c6`.

.. only:: esp32c61

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15 20

        * - Type/Throughput
          - Air In Lab
          - Shield-box
          - Test Tool
          - IDF Version (commit ID)
        * - Raw 802.11 Packet RX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - Raw 802.11 Packet TX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - UDP RX
          - 30 MBit/s
          - 68 MBit/s
          - iperf example
          - 7ff0a07d
        * - UDP TX
          - 30 MBit/s
          - 53 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP RX
          - 20 MBit/s
          - 45 MBit/s
          - iperf example
          - 7ff0a07d
        * - TCP TX
          - 20 MBit/s
          - 37 MBit/s
          - iperf example
          - 7ff0a07d

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32c61`.

.. only:: esp32s3

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15 20

        * - Type/Throughput
          - Air In Lab
          - Shield-box
          - Test Tool
          - IDF Version (commit ID)
        * - Raw 802.11 Packet RX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - Raw 802.11 Packet TX
          - N/A
          - **130 MBit/s**
          - Internal tool
          - NA
        * - UDP RX
          - 30 MBit/s
          - 88 MBit/s
          - iperf example
          - 15575346
        * - UDP TX
          - 30 MBit/s
          - 98 MBit/s
          - iperf example
          - 15575346
        * - TCP RX
          - 20 MBit/s
          - 73 MBit/s
          - iperf example
          - 15575346
        * - TCP TX
          - 20 MBit/s
          - 83 MBit/s
          - iperf example
          - 15575346

    When the throughput is tested by iperf example, the sdkconfig is :idf_file:`examples/wifi/iperf/sdkconfig.defaults.esp32s3`.

Wi-Fi 80211 Packet Send
---------------------------

The :cpp:func:`esp_wifi_80211_tx()` API can be used to:

 - Send the beacon, probe request, probe response, and action frame.
 - Send the non-QoS data frame.

It cannot be used for sending encrypted or QoS frames.

Preconditions of Using :cpp:func:`esp_wifi_80211_tx()`
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 - The Wi-Fi mode is station, or AP, or station/AP.
 - Either ``esp_wifi_set_promiscuous(true)``, or :cpp:func:`esp_wifi_start()`, or both of these APIs return :c:macro:`ESP_OK`. This is because Wi-Fi hardware must be initialized before :cpp:func:`esp_wifi_80211_tx()` is called. In {IDF_TARGET_NAME}, both ``esp_wifi_set_promiscuous(true)`` and :cpp:func:`esp_wifi_start()` can trigger the initialization of Wi-Fi hardware.
 - The parameters of :cpp:func:`esp_wifi_80211_tx()` are hereby correctly provided.

Data Rate
+++++++++++++++++++++++++++++++++++++++++++++++

 - The default data rate is 1 Mbps.
 - Can set any rate through :cpp:func:`esp_wifi_config_80211_tx_rate()` API.
 - Can set any bandwidth through :cpp:func:`esp_wifi_set_bandwidth()` API.

Side-Effects to Avoid in Different Scenarios
+++++++++++++++++++++++++++++++++++++++++++++++++++++

Theoretically, if the side-effects the API imposes on the Wi-Fi driver or other stations/APs are not considered, a raw 802.11 packet can be sent over the air with any destination MAC, any source MAC, any BSSID, or any other types of packet. However, robust or useful applications should avoid such side-effects. The table below provides some tips and recommendations on how to avoid the side-effects of :cpp:func:`esp_wifi_80211_tx()` in different scenarios.

.. list-table::
   :header-rows: 1
   :widths: 10 50

   * - Scenario
     - Description
   * - No Wi-Fi connection
     - In this scenario, no Wi-Fi connection is set up, so there are no side-effects on the Wi-Fi driver. If ``en_sys_seq==true``, the Wi-Fi driver is responsible for the sequence control. If ``en_sys_seq==false``, the application needs to ensure that the buffer has the correct sequence.

       Theoretically, the MAC address can be any address. However, this may impact other stations/APs with the same MAC/BSSID.

       Side-effect example#1 The application calls :cpp:func:`esp_wifi_80211_tx()` to send a beacon with BSSID == mac_x in AP mode, but the mac_x is not the MAC of the AP interface. Moreover, there is another AP, e.g., “other-AP”, whose BSSID is mac_x. If this happens, an “unexpected behavior” may occur, because the stations which connect to the “other-AP” cannot figure out whether the beacon is from the “other-AP” or the :cpp:func:`esp_wifi_80211_tx()`.

       To avoid the above-mentioned side-effects, it is recommended that:

       - If :cpp:func:`esp_wifi_80211_tx` is called in station mode, the first MAC should be a multicast MAC or the exact target-device’s MAC, while the second MAC should be that of the station interface.

       - If :cpp:func:`esp_wifi_80211_tx` is called in AP mode, the first MAC should be a multicast MAC or the exact target-device’s MAC, while the second MAC should be that of the AP interface.

       The recommendations above are only for avoiding side-effects and can be ignored when there are good reasons.

   * - Have Wi-Fi connection
     - When the Wi-Fi connection is already set up, and the sequence is controlled by the application, the latter may impact the sequence control of the Wi-Fi connection as a whole. So, the ``en_sys_seq`` need to be true, otherwise ``ESP_ERR_INVALID_ARG`` is returned.

       The MAC-address recommendations in the “No Wi-Fi connection” scenario also apply to this scenario.

       If the Wi-Fi mode is station mode, the MAC address1 is the MAC of AP to which the station is connected, and the MAC address2 is the MAC of station interface, it is said that the packet is sent from the station to AP. Otherwise, if the Wi-Fi is in AP mode, the MAC address1 is the MAC of the station that connects to this AP, and the MAC address2 is the MAC of AP interface, it is said that the packet is sent from the AP to station. To avoid conflicting with Wi-Fi connections, the following checks are applied:

       - If the packet type is data and is sent from the station to AP, the ToDS bit in IEEE 80211 frame control should be 1 and the FromDS bit should be 0. Otherwise, the packet will be discarded by Wi-Fi driver.

       - If the packet type is data and is sent from the AP to station, the ToDS bit in IEEE 80211 frame control should be 0 and the FromDS bit should be 1. Otherwise, the packet will be discarded by Wi-Fi driver.

       - If the packet is sent from station to AP or from AP to station, the Power Management, More Data, and Re-Transmission bits should be 0. Otherwise, the packet will be discarded by Wi-Fi driver.

       ``ESP_ERR_INVALID_ARG`` is returned if any check fails.


Wi-Fi Sniffer Mode
---------------------------

The Wi-Fi sniffer mode can be enabled by :cpp:func:`esp_wifi_set_promiscuous`. If the sniffer mode is enabled, the following packets **can** be dumped to the application:

 - 802.11 Management frame.
 - 802.11 Data frame, including MPDU, AMPDU, and AMSDU.
 - 802.11 MIMO frame, for MIMO frame, the sniffer only dumps the length of the frame.
 - 802.11 Control frame.
 - 802.11 CRC error frame.

The following packets will **NOT** be dumped to the application:

 - Other 802.11 error frames.

For frames that the sniffer **can** dump, the application can additionally decide which specific type of packets can be filtered to the application by using :cpp:func:`esp_wifi_set_promiscuous_filter()` and :cpp:func:`esp_wifi_set_promiscuous_ctrl_filter()`. By default, it will filter all 802.11 data and management frames to the application. If you want to filter the 802.11 control frames, the filter parameter in :cpp:func:`esp_wifi_set_promiscuous_filter()` should include `WIFI_PROMIS_FILTER_MASK_CTRL` type, and if you want to differentiate control frames further, then call :cpp:func:`esp_wifi_set_promiscuous_ctrl_filter()`.

The Wi-Fi sniffer mode can be enabled in the Wi-Fi mode of :cpp:enumerator:`WIFI_MODE_NULL`, :cpp:enumerator:`WIFI_MODE_STA`, :cpp:enumerator:`WIFI_MODE_AP`, or :cpp:enumerator:`WIFI_MODE_APSTA`. In other words, the sniffer mode is active when the station is connected to the AP, or when the AP has a Wi-Fi connection. Please note that the sniffer has a **great impact** on the throughput of the station or AP Wi-Fi connection. Generally, the sniffer should be enabled **only if** the station/AP Wi-Fi connection does not experience heavy traffic.

Another noteworthy issue about the sniffer is the callback :cpp:type:`wifi_promiscuous_cb_t`. The callback will be called directly in the Wi-Fi driver task, so if the application has a lot of work to do for each filtered packet, the recommendation is to post an event to the application task in the callback and defer the real work to the application task.

Wi-Fi Multiple Antennas
---------------------------
Please refer to the :doc:`PHY <../api-guides/phy>`

.. only:: SOC_WIFI_CSI_SUPPORT

    Wi-Fi Channel State Information
    ------------------------------------

    .. only:: esp32 or esp32s2 or esp32c3 or esp32s3

        Channel state information (CSI) refers to the channel information of a Wi-Fi connection. In {IDF_TARGET_NAME}, this information consists of channel frequency responses of sub-carriers and is estimated when packets are received from the transmitter. Each channel frequency response of sub-carrier is recorded by two bytes of signed characters. The first one is imaginary part and the second one is real part. There are up to three fields of channel frequency responses according to the type of received packet. They are legacy long training field (LLTF), high throughput LTF (HT-LTF), and space time block code HT-LTF (STBC-HT-LTF). For different types of packets which are received on channels with different state, the sub-carrier index and total bytes of signed characters of CSI are shown in the following table.

        +-------------+--------------------+-----------------------------------------+--------------------------------------------------------+----------------------------------------------------------+
        | channel     | secondary channel  |                   none                  |                           below                        |                            above                         |
        +-------------+--------------------+-------------+---------------------------+----------+---------------------------------------------+----------+-----------------------------------------------+
        | packet      | signal mode        |   non HT    |            HT             |  non HT  |                      HT                     |  non HT  |                       HT                      |
        +             +--------------------+-------------+---------------------------+----------+-----------------+---------------------------+----------+-------------------+---------------------------+
        | information | channel bandwidth  |    20 MHz   |           20 MHz          |   20 MHz |      20 MHz     |            40 MHz         |   20 MHz |       20 MHz      |            40 MHz         |
        +             +--------------------+-------------+-------------+-------------+----------+----------+------+-------------+-------------+----------+----------+--------+-------------+-------------+
        |             | STBC               |  non STBC   |  non STBC   |     STBC    | non STBC | non STBC | STBC |  non STBC   |     STBC    | non STBC | non STBC |  STBC  |  non STBC   |     STBC    |
        +-------------+--------------------+-------------+-------------+-------------+----------+----------+------+-------------+-------------+----------+----------+--------+-------------+-------------+
        | sub-carrier | LLTF               | 0~31, -32~-1| 0~31, -32~-1| 0~31, -32~-1|   0~63   |   0~63   | 0~63 |     0~63    |     0~63    |  -64~-1  |  -64~-1  | -64~-1 |    -64~-1   |    -64~-1   |
        +             +--------------------+-------------+-------------+-------------+----------+----------+------+-------------+-------------+----------+----------+--------+-------------+-------------+
        | index       | HT-LTF             |      -      | 0~31, -32~-1| 0~31, -32~-1|     -    |   0~63   | 0~62 | 0~63, -64~-1| 0~60, -60~-1|     -    |  -64~-1  | -62~-1 | 0~63, -64~-1| 0~60, -60~-1|
        +             +--------------------+-------------+-------------+-------------+----------+----------+------+-------------+-------------+----------+----------+--------+-------------+-------------+
        |             | STBC-HT-LTF        |      -      |      -      | 0~31, -32~-1|     -    |     -    | 0~62 |       -     | 0~60, -60~-1|     -    |     -    | -62~-1 |       -     | 0~60, -60~-1|
        +-------------+--------------------+-------------+-------------+-------------+----------+----------+------+-------------+-------------+----------+----------+--------+-------------+-------------+
        | total bytes                      |     128     |     256     |     384     |    128   |    256   | 380  |      384    |      612    |    128   |    256   |   376  |      384    |      612    |
        +----------------------------------+-------------+-------------+-------------+----------+----------+------+-------------+-------------+----------+----------+--------+-------------+-------------+

        All of the information in the table can be found in the structure wifi_csi_info_t.

            - Secondary channel refers to secondary_channel field of rx_ctrl field.
            - Signal mode of packet refers to sig_mode field of rx_ctrl field.
            - Channel bandwidth refers to cwb field of rx_ctrl field.
            - STBC refers to stbc field of rx_ctrl field.
            - Total bytes refers to len field.
            - The CSI data corresponding to each Long Training Field (LTF) type is stored in a buffer starting from the buf field. Each item is stored as two bytes: imaginary part followed by real part. The order of each item is the same as the sub-carrier in the table. The order of LTF is: LLTF, HT-LTF, STBC-HT-LTF. However, all 3 LTFs may not be present, depending on the channel and packet information (see above).
            - If first_word_invalid field of :cpp:type:`wifi_csi_info_t` is true, it means that the first four bytes of CSI data is invalid due to a hardware limitation in {IDF_TARGET_NAME}.
            - More information like RSSI, noise floor of RF, receiving time and antenna is in the rx_ctrl field.

        When imaginary part and real part data of sub-carrier are used, please refer to the table below.

        +-----------------+-------------------+------------------------------+--------------------------+
        | PHY standard    | Sub-carrier range | Pilot sub-carrier            | Sub-carrier (total/data) |
        +=================+===================+==============================+==========================+
        | 802.11a/g       | -26 to +26        | -21, -7, +7, +21             | 52 total, 48 usable      |
        +-----------------+-------------------+------------------------------+--------------------------+
        | 802.11n, 20 MHz | -28 to +28        | -21, -7, +7, +21             | 56 total, 52 usable      |
        +-----------------+-------------------+------------------------------+--------------------------+
        | 802.11n, 40 MHz | -57 to +57        | -53, -25, -11, +11, +25, +53 | 114 total, 108 usable    |
        +-----------------+-------------------+------------------------------+--------------------------+

        .. note::

            - For STBC packet, CSI is provided for every space-time stream without CSD (cyclic shift delay). As each cyclic shift on the additional chains shall be -200 ns, only the CSD angle of first space-time stream is recorded in sub-carrier 0 of HT-LTF and STBC-HT-LTF for there is no channel frequency response in sub-carrier 0. CSD[10:0] is 11 bits, ranging from -pi to pi.

            - If LLTF, HT-LTF, or STBC-HT-LTF is not enabled by calling API :cpp:func:`esp_wifi_set_csi_config()`, the total bytes of CSI data will be fewer than that in the table. For example, if LLTF and HT-LTF is not enabled and STBC-HT-LTF is enabled, when a packet is received with the condition above/HT/40MHz/STBC, the total bytes of CSI data is 244 ((61 + 60) * 2 + 2 = 244. The result is aligned to four bytes, and the last two bytes are invalid).

    .. only:: esp32c5

        Channel state information (CSI) refers to the channel information of a Wi-Fi connection. In {IDF_TARGET_NAME}, this information consists of channel frequency responses of sub-carriers and is estimated when packets are received from the transmitter. Each channel frequency response of sub-carrier is recorded by two bytes of signed characters. The first one is imaginary part and the second one is real part. Except for the IEEE 802.11g mode, all other modes have two LTF sequences (LLTF + HT/VHT/HE-LTF). {IDF_TARGET_NAME} can determine whether to include LLTF or HT/VHT/HE-LTF through ``acquire_csi_force_lltf`` field of :cpp:struct:`wifi_csi_acquire_config_t`. For different types of packets which are received on channels with different state, the sub-carrier index and total bytes of signed characters of CSI are shown in the following table.

        +-------------+-------------------+--------------------------------------------+-----------------------------------+--------------------------------------------------------------------------+--------------------------------------------------------------------------+
        | channel     | secondary channel |                                           none                                 |                                  below                                   |                                  above                                   |
        +-------------+-------------------+--------------+-----------------------------+-----------------------------------+--------------+-----------------------------------------------------------+--------------+-----------------------------------------------------------+
        | packet      | signal mode       |    non HT    |              HT             |                 HE                |    non HT    |                           HT                              |    non HT    |                           HT                              |
        |             +-------------------+--------------+-----------------------------+-----------------------------------+--------------+-----------------------------+-----------------------------+--------------+-----------------------------+-----------------------------+
        | information | channel bandwidth |    20 MHz    |            20 MHz           |               20 MHz              |    20 MHz    |            20 MHz           |            40 MHz           |    20 MHz    |             20 MHz          |           40 MHz            |
        |             +-------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        |             | STBC              |   non STBC   |   non STBC   |     STBC     |    non STBC   |       STBC        |   non STBC   |   non STBC   |     STBC     |   none STBC  |     STBC     |   non STBC   |   non STBC   |     STBC     |   non STBC   |     STBC     |
        +-------------+-------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        | sub-carrier | LLTF              | 0~26, -26~-1 |      —       |      —       |       —       |         —         |     0~52     |      —       |      —       |      —       |      —       |    -53~-1    |       —      |      —       |      —       |      —       |
        |             +-------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        | index       | HT-LTF (HT-LTF1)  |      —       | 0~28, -28~-1 | 0~28, -28~-1 |       —       |         —         |      —       |     0~56     |     0~56     | 0~58, -58~-1 | 0~58, -58~-1 |       —      |    -57~-1    |    -57~-1    | 0~58, -58~-1 | 0~58, -58~-1 |
        |             +-------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        |             | HT-LTF2           |      —       |      —       | 0~28, -28~-1 |       —       |         —         |      —       |      —       |     0~56     |      —       | 0~58, -58~-1 |       —      |       —      |    -57~-1    |      —       | 0~58, -58~-1 |
        |             +-------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        |             | HE-LTF (HE-LTF1)  |      —       |      —       |      —       | 0~122, -122~1 | Determined by     |      —       |      —       |      —       |      —       |      —       |       —      |       —      |      —       |      —       |      —       |
        |             +-------------------+--------------+--------------+--------------+---------------+                   +--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        |             | HE-LTF2           |      —       |      —       |      —       |       —       | wifi_csi_config_t |      —       |      —       |      —       |      —       |      —       |       —      |       —      |      —       |      —       |      —       |
        +-------------+-------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+
        | total bytes                     |     106      |     114      |     228      |      490      |        490        |     106      |     114      |     228      |     234      |     468      |      106     |      114     |     228      |     234      |     468      |
        +---------------------------------+--------------+--------------+--------------+---------------+-------------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+--------------+

        .. note ::

            - In HT/VHT/HE modes, there are two LTF sequences: LLTF + HT/VHT/HE-LTF. If the ``acquire_csi_force_lltf`` field of :cpp:struct:`wifi_csi_acquire_config_t` is set to false, the CSI data will only contain HT/VHT/HE-LTF (as shown in the table above); otherwise, the CSI data will only contain LLTF. The sub-carrier index and total bytes for LLTF in HT/VHT/HE modes are the same as those for LLTF in non-HT modes.
            - In VHT mode, the sub-carrier index and total bytes are the same as those in HT mode.

        All of the information in the table can be found in the structure :cpp:type:`wifi_csi_info_t`.

            - Secondary channel refers to ``second`` field of ``rx_ctrl`` field.
            - signal mode of packet refers to  ``cur_bb_format`` field of ``rx_ctrl`` field.
            - total bytes refers to ``len`` field
            - The CSI data corresponding to each Long Training Field (LTF) type is stored in a buffer starting from the buf field. Each item is stored as two bytes: imaginary part followed by real part. The order of each item is the same as the sub-carrier in the table.
            - If ``first_word_invalid`` of :cpp:type:`wifi_csi_info_t` is true, it means that the first four bytes of CSI data is invalid due to a hardware limitation in {IDF_TARGET_NAME}.
            - If ``rx_channel_estimate_info_vld`` of ``rx_ctrl`` field is 1, indicates that the CSI data is valid; otherwise, the CSI data is invalid.
            - More information like RSSI, noise floor of RF, receiving time and antenna is in the ``rx_ctrl`` field.

        For STBC packets, the subcarrier indices of HE-LTF1 and HE-LTF2 are determined by ``acquire_csi_he_stbc_mode`` field of :cpp:type:`wifi_csi_config_t`. Please refer to the table below for details.

        +---------------------+----------------------+----------------------+
        | acquire_csi_he_stbc |         HE-LTF1      |         HE-LTF2      |
        +---------------------+----------------------+----------------------+
        |         0           |    -122~-1, 0~122    |           —          |
        +---------------------+----------------------+----------------------+
        |         1           |           —          |    -122~-1, 0~122    |
        +---------------------+----------------------+----------------------+
        |         2           | Sample evenly among the HE-LTF1 and HE-LTF2 |
        +---------------------+----------------------+----------------------+

        When imaginary part and real part data of sub-carrier are used, please refer to the table below.

        +-----------------------+-------------------+------------------------------------------+------------------------------+
        | PHY standard          | Sub-carrier range |           Invalid sub-carrier            |   Sub-carrier (total/data)   |
        +=======================+===================+==========================================+==============================+
        | 802.11a/g             |  -26 to +26       |                    0                     |     53 total, 52 usable      |
        +-----------------------+-------------------+------------------------------------------+------------------------------+
        | 802.11n, 20 MHz       |  -28 to +28       |                    0                     |     57 total, 56 usable      |
        +-----------------------+-------------------+------------------------------------------+------------------------------+
        | 802.11n, 40 MHz       |  -58 to +58       |                -1, 0, 1                  |     117 total, 114 usable    |
        +-----------------------+-------------------+------------------------------------------+------------------------------+
        | 802.11ac, 20 MHz      |  -28 to +28       |                    0                     |     57 total, 56 usable      |
        +-----------------------+-------------------+------------------------------------------+------------------------------+
        | 802.11ax, 20 MHz (SU) |  -122 to + 122    |                -1, 0, 1                  |     245 total, 242 usable    |
        +-----------------------+-------------------+------------------------------------------+------------------------------+

        .. note::

            - When the PHY is 802.11ax, please refer to the protocol for sub-carrier range and invalid sub-carrier for MU packets.

        .. note::

            - For STBC packet, CSI is provided for every space-time stream without CSD (cyclic shift delay). As each cyclic shift on the additional chains shall be -200 ns, only the CSD angle of first space-time stream is recorded in sub-carrier 0 of HT-LTF1 (HE-LTF1) and HT-LTF2 (HE-LTF2) for there is no channel frequency response in sub-carrier 0. CSD[10:0] is 11 bits, ranging from -pi to pi.

    Wi-Fi Channel State Information Configure
    -------------------------------------------

    To use Wi-Fi CSI, the following steps need to be done.

        - Select Wi-Fi CSI in menuconfig. Go to ``Menuconfig`` > ``Components config`` > ``Wi-Fi`` > ``Wi-Fi CSI (Channel State Information)``.
        - Set CSI receiving callback function by calling API :cpp:func:`esp_wifi_set_csi_rx_cb()`.
        - Configure CSI by calling API :cpp:func:`esp_wifi_set_csi_config()`.
        - Enable CSI by calling API :cpp:func:`esp_wifi_set_csi()`.

    The CSI receiving callback function runs from Wi-Fi task. So, do not do lengthy operations in the callback function. Instead, post necessary data to a queue and handle it from a lower priority task. Because station does not receive any packet when it is disconnected and only receives packets from AP when it is connected, it is suggested to enable sniffer mode to receive more CSI data by calling :cpp:func:`esp_wifi_set_promiscuous()`.

Wi-Fi HT20/40
-------------------------

.. only:: esp32 or esp32s2 or esp32c3 or esp32s3 or esp32c5 or esp32c6

    {IDF_TARGET_NAME} supports Wi-Fi bandwidth HT20 or HT40 and does not support HT20/40 coexist. :cpp:func:`esp_wifi_set_bandwidth()` can be used to change the default bandwidth of station or AP. The default bandwidth for {IDF_TARGET_NAME} station and AP is HT40.

    In station mode, the actual bandwidth is firstly negotiated during the Wi-Fi connection. It is HT40 only if both the station and the connected AP support HT40, otherwise it is HT20. If the bandwidth of connected AP is changes, the actual bandwidth is negotiated again without Wi-Fi disconnecting.

    Similarly, in AP mode, the actual bandwidth is negotiated between AP and the stations that connect to the AP. It is HT40 if the AP and one of the stations support HT40, otherwise it is HT20.

    In station/AP coexist mode, the station/AP can configure HT20/40 separately. If both station and AP are negotiated to HT40, the HT40 channel should be the channel of station because the station always has higher priority than AP in {IDF_TARGET_NAME}. For example, the configured bandwidth of AP is HT40, the configured primary channel is 6, and the configured secondary channel is 10. The station is connected to an router whose primary channel is 6 and secondary channel is 2, then the actual channel of AP is changed to primary 6 and secondary 2 automatically.

    Theoretically, the HT40 can gain better throughput because the maximum raw physical (PHY) data rate for HT40 is 150 Mbps while it is 72 Mbps for HT20. However, if the device is used in some special environment, e.g., there are too many other Wi-Fi devices around the {IDF_TARGET_NAME} device, the performance of HT40 may be degraded. So if the applications need to support same or similar scenarios, it is recommended that the bandwidth is always configured to HT20.

.. only:: esp32c5

    ..note ..

        When operating in the 2.4 GHz + 5 GHz band mode (``WIFI_BAND_MODE_AUTO``), can use the function :cpp:func:`esp_wifi_set_bandwidths()` to configure the bandwidth for the 2.4 GHz and 5 GHz bands separately.

.. only:: esp32c2

    {IDF_TARGET_NAME} supports Wi-Fi bandwidth HT20 and does not support Wi-Fi bandwidth HT40 or HT20/40 coexist.

Wi-Fi QoS
-------------------------

{IDF_TARGET_NAME} supports all the mandatory features required in WFA Wi-Fi QoS Certification.

Four ACs (Access Category) are defined in Wi-Fi specification, and each AC has its own priority to access the Wi-Fi channel. Moreover, a map rule is defined to map the QoS priority of other protocol, e.g., 802.11D or TCP/IP precedence is mapped to Wi-Fi AC.

The table below describes how the IP Precedences are mapped to Wi-Fi ACs in {IDF_TARGET_NAME}. It also indicates whether the AMPDU is supported for this AC. The table is sorted from high to low priority. That is to say, the AC_VO has the highest priority.

+------------------+------------------------+-----------------+
| IP Precedence    | Wi-Fi AC               |  Support AMPDU? |
+==================+========================+=================+
| 6, 7             | AC_VO (Voice)          |  No             |
+------------------+------------------------+-----------------+
| 4, 5             | AC_VI (Video)          |  Yes            |
+------------------+------------------------+-----------------+
| 3, 0             | AC_BE (Best Effort)    |  Yes            |
+------------------+------------------------+-----------------+
| 1, 2             | AC_BK (Background)     |  Yes            |
+------------------+------------------------+-----------------+

The application can make use of the QoS feature by configuring the IP precedence via socket option IP_TOS. Here is an example to make the socket to use VI queue::

    const int ip_precedence_vi = 4;
    const int ip_precedence_offset = 5;
    int priority = (ip_precedence_vi << ip_precedence_offset);
    setsockopt(socket_id, IPPROTO_IP, IP_TOS, &priority, sizeof(priority));

Theoretically, the higher priority AC has better performance than the lower priority AC. However, it is not always true. Here are some suggestions about how to use the Wi-Fi QoS:

 - Some really important application traffic can be put into the AC_VO queue. But avoid using the AC_VO queue for heavy traffic, as it may impact the management frames which also use this queue. Eventually, it is worth noting that the AC_VO queue does not support AMPDU, and its performance with heavy traffic is no better than other queues.
 - Avoid using more than two precedences supported by different AMPDUs, e.g., when socket A uses precedence 0, socket B uses precedence 1, and socket C uses precedence 2. This can be a bad design because it may need much more memory. To be specific, the Wi-Fi driver may generate a Block Ack session for each precedence and it needs more memory if the Block Ack session is set up.


Wi-Fi AMSDU
-------------------------

.. only:: not SOC_SPIRAM_SUPPORTED

    {IDF_TARGET_NAME} supports receiving AMSDU.

.. only:: SOC_SPIRAM_SUPPORTED

    {IDF_TARGET_NAME} supports receiving and transmitting AMSDU. AMSDU TX is disabled by default, since enable AMSDU TX need more memory. Select :ref:`CONFIG_ESP_WIFI_AMSDU_TX_ENABLED` to enable AMSDU Tx feature, it depends on :ref:`CONFIG_SPIRAM`.

Wi-Fi Fragment
-------------------------

.. only:: esp32 or esp32s2

    supports Wi-Fi receiving fragment, but does not support Wi-Fi transmitting fragment.

.. only:: esp32c3 or esp32s3 or esp32c5 or esp32c6

    {IDF_TARGET_NAME} supports Wi-Fi receiving and transmitting fragment.

WPS Enrollee
-------------------------

{IDF_TARGET_NAME} supports WPS enrollee feature in Wi-Fi mode :cpp:enumerator:`WIFI_MODE_STA` or :cpp:enumerator:`WIFI_MODE_APSTA`. Currently, {IDF_TARGET_NAME} supports WPS enrollee type PBC and PIN.

.. _wifi-buffer-usage:

Wi-Fi Buffer Usage
--------------------------

This section is only about the dynamic buffer configuration.

Why Buffer Configuration Is Important
+++++++++++++++++++++++++++++++++++++++

In order to get a high-performance system, consider the memory usage/configuration carefully for the following reasons:

 - the available memory in {IDF_TARGET_NAME} is limited.
 - currently, the default type of buffer in LwIP and Wi-Fi drivers is "dynamic", **which means that both the LwIP and Wi-Fi share memory with the application**. Programmers should always keep this in mind; otherwise, they will face a memory issue, such as "running out of heap memory".
 - it is very dangerous to run out of heap memory, as this will cause {IDF_TARGET_NAME} an "undefined behavior". Thus, enough heap memory should be reserved for the application, so that it never runs out of it.
 - the Wi-Fi throughput heavily depends on memory-related configurations, such as the TCP window size and Wi-Fi RX/TX dynamic buffer number.
 - the peak heap memory that the {IDF_TARGET_NAME} LwIP/Wi-Fi may consume depends on a number of factors, such as the maximum TCP/UDP connections that the application may have.
 - the total memory that the application requires is also an important factor when considering memory configuration.

Due to these reasons, there is not a good-for-all application configuration. Rather, it is recommended to consider memory configurations separately for every different application.

Dynamic vs. Static Buffer
++++++++++++++++++++++++++++++

The default type of buffer in Wi-Fi drivers is "dynamic". Most of the time the dynamic buffer can significantly save memory. However, it makes the application programming a little more difficult, because in this case the application needs to consider memory usage in Wi-Fi.

lwIP also allocates buffers at the TCP/IP layer, and this buffer allocation is also dynamic. See :ref:`lwIP documentation section about memory use and performance <lwip-performance>`.

Peak Wi-Fi Dynamic Buffer
++++++++++++++++++++++++++++++

The Wi-Fi driver supports several types of buffer (refer to `Wi-Fi Buffer Configure`_). However, this section is about the usage of the dynamic Wi-Fi buffer only.
The peak heap memory that Wi-Fi consumes is the **theoretically-maximum memory** that the Wi-Fi driver consumes. Generally, the peak memory depends on:

- :math:`b_{rx}` the number of dynamic RX buffers that are configured
- :math:`b_{tx}` the number of dynamic TX buffers that are configured
- :math:`m_{rx}` the maximum packet size that the Wi-Fi driver can receive
- :math:`m_{tx}` the maximum packet size that the Wi-Fi driver can send

So, the peak memory that the Wi-Fi driver consumes (:math:`p`) can be calculated with the following formula:

.. math::

    p = (b_{rx} * m_{rx}) + (b_{tx} * m_{tx})

Generally, the dynamic TX long buffers and dynamic TX long long buffers can be ignored, because they are management frames which only have a small impact on the system.

.. _How-to-improve-Wi-Fi-performance:

How to Improve Wi-Fi Performance
----------------------------------

The performance of {IDF_TARGET_NAME} Wi-Fi is affected by many parameters, and there are mutual constraints between each parameter. A proper configuration cannot only improve performance, but also increase available memory for applications and improve stability.

This section briefly explains the operating mode of the Wi-Fi/LwIP protocol stack and the role of each parameter. It also gives several recommended configuration ranks to help choose the appropriate rank according to the usage scenario.

Protocol Stack Operation Mode
++++++++++++++++++++++++++++++++++

.. figure:: ../../_static/api-guides-WiFi-driver-how-to-improve-WiFi-performance.png
    :align: center

    {IDF_TARGET_NAME} datapath

The {IDF_TARGET_NAME} protocol stack is divided into four layers: Application, LwIP, Wi-Fi, and Hardware.

 - During receiving, hardware puts the received packet into DMA buffer, and then transfers it into the RX buffer of Wi-Fi and LwIP in turn for related protocol processing, and finally to the application layer. The Wi-Fi RX buffer and the LwIP RX buffer shares the same buffer by default. In other words, the Wi-Fi forwards the packet to LwIP by reference by default.

 - During sending, the application copies the messages to be sent into the TX buffer of the LwIP layer for TCP/IP encapsulation. The messages will then be passed to the TX buffer of the Wi-Fi layer for MAC encapsulation and wait to be sent.

Parameters
++++++++++++++

Increasing the size or number of the buffers mentioned above properly can improve Wi-Fi performance. Meanwhile, it will reduce available memory to the application. The following is an introduction to the parameters that users need to configure:

**RX direction:**

 - :ref:`CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM`
    This parameter indicates the number of DMA buffer at the hardware layer. Increasing this parameter will increase the sender's one-time receiving throughput, thereby improving the Wi-Fi protocol stack ability to handle burst traffic.

 - :ref:`CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM`
    This parameter indicates the number of RX buffer in the Wi-Fi layer. Increasing this parameter will improve the performance of packet reception. This parameter needs to match the RX buffer size of the LwIP layer.

 - :ref:`CONFIG_ESP_WIFI_RX_BA_WIN`
    This parameter indicates the size of the AMPDU BA Window at the receiving end. This parameter should be configured to the smaller value between twice of :ref:`CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM` and :ref:`CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM`.

 - :ref:`CONFIG_LWIP_TCP_WND_DEFAULT`
    This parameter represents the RX buffer size of the LwIP layer for each TCP stream. Its value should be configured to the value of WIFI_DYNAMIC_RX_BUFFER_NUM (KB) to reach a high and stable performance. Meanwhile, in case of multiple streams, this value needs to be reduced proportionally.

**TX direction:**

 - :ref:`CONFIG_ESP_WIFI_TX_BUFFER`
    This parameter indicates the type of TX buffer, it is recommended to configure it as a dynamic buffer, which can make full use of memory.

 - :ref:`CONFIG_ESP_WIFI_DYNAMIC_TX_BUFFER_NUM`
    This parameter indicates the number of TX buffer on the Wi-Fi layer. Increasing this parameter will improve the performance of packet sending. The parameter value needs to match the TX buffer size of the LwIP layer.

 - :ref:`CONFIG_LWIP_TCP_SND_BUF_DEFAULT`
    This parameter represents the TX buffer size of the LwIP layer for each TCP stream. Its value should be configured to the value of WIFI_DYNAMIC_TX_BUFFER_NUM (KB) to reach a high and stable performance. In case of multiple streams, this value needs to be reduced proportionally.

**Throughput optimization by placing code in IRAM:**

.. only:: esp32 or esp32s2

    - :ref:`CONFIG_ESP_WIFI_IRAM_OPT`
        If this option is enabled, some Wi-Fi functions are moved to IRAM, improving throughput. This increases IRAM usage by 15 kB.

    - :ref:`CONFIG_ESP_WIFI_RX_IRAM_OPT`
        If this option is enabled, some Wi-Fi RX functions are moved to IRAM, improving throughput. This increases IRAM usage by 16 kB.

 - :ref:`CONFIG_LWIP_IRAM_OPTIMIZATION`
    If this option is enabled, some LwIP functions are moved to IRAM, improving throughput. This increases IRAM usage by 13 kB.

.. only:: esp32c6

    - :ref:`CONFIG_ESP_WIFI_IRAM_OPT`
        If this option is enabled, some Wi-Fi functions are moved to IRAM, improving throughput. This increases IRAM usage by 13 kB.

    - :ref:`CONFIG_ESP_WIFI_RX_IRAM_OPT`
        If this option is enabled, some Wi-Fi RX functions are moved to IRAM, improving throughput. This increases IRAM usage by 7 kB.

 - :ref:`CONFIG_LWIP_IRAM_OPTIMIZATION`
    If this option is enabled, some LwIP functions are moved to IRAM, improving throughput. This increases IRAM usage by 14 kB.

.. only:: esp32s2

    **CACHE:**

     - :ref:`CONFIG_ESP32S2_INSTRUCTION_CACHE_SIZE`
        Configure the size of the instruction Cache.

     - :ref:`CONFIG_ESP32S2_INSTRUCTION_CACHE_LINE_SIZE`
        Configure the width of the instruction Cache bus.

.. only:: esp32s3

    **CACHE:**

     - :ref:`CONFIG_ESP32S3_INSTRUCTION_CACHE_SIZE`
        Configure the size of the instruction Cache.

     - :ref:`CONFIG_ESP32S3_INSTRUCTION_CACHE_LINE_SIZE`
        Configure the size of the instruction Cache bus.

     - :ref:`CONFIG_ESP32S3_ICACHE_ASSOCIATED_WAYS`
        Configure the associated ways of the instruction Cache.

     - :ref:`CONFIG_ESP32S3_DATA_CACHE_SIZE`
        Configure the size of the Data Cache.

     - :ref:`CONFIG_ESP32S3_DATA_CACHE_LINE_SIZE`
        Configure the line size of the Data Cache.

     - :ref:`CONFIG_ESP32S3_DCACHE_ASSOCIATED_WAYS`
        Configure the associated ways of the Data Cache.

.. note::
    The buffer size mentioned above is fixed as 1.6 KB.

How to Configure Parameters
++++++++++++++++++++++++++++

The memory of {IDF_TARGET_NAME} is shared by protocol stack and applications.

Here, several configuration ranks are given. In most cases, the user should select a suitable rank for parameter configuration according to the size of the memory occupied by the application.

The parameters not mentioned in the following table should be set to the default.

.. only:: esp32

     .. list-table::
        :header-rows: 1
        :widths: 10 5 5 10 5 5 10 5

        * - Rank
          - Iperf
          - TX prior
          - High-performance
          - RX prior
          - Default
          - Memory saving
          - Minimum
        * - Available memory (KB)
          - 37.1
          - 113.8
          - 123.3
          - 145.5
          - 144.5
          - 170.2
          - 185.2
        * - WIFI_STATIC_RX_BUFFER_NUM
          - 16
          - 6
          - 6
          - 6
          - 6
          - 6
          - 4
        * - WIFI_DYNAMIC_RX_BUFFER_NUM
          - 64
          - 16
          - 24
          - 34
          - 20
          - 12
          - 8
        * - WIFI_DYNAMIC_TX_BUFFER_NUM
          - 64
          - 28
          - 24
          - 18
          - 20
          - 12
          - 8
        * - WIFI_RX_BA_WIN
          - 32
          - 8
          - 12
          - 12
          - 10
          - 6
          - Disable
        * - TCP_SND_BUF_DEFAULT (KB)
          - 65
          - 28
          - 24
          - 18
          - 20
          - 12
          - 8
        * - TCP_WND_DEFAULT (KB)
          - 65
          - 16
          - 24
          - 34
          - 20
          - 12
          - 8
        * - WIFI_IRAM_OPT
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
        * - WIFI_RX_IRAM_OPT
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
        * - LWIP_IRAM_OPTIMIZATION
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
        * - TCP TX throughput (Mbit/s)
          - 74.6
          - 50.8
          - 46.5
          - 39.9
          - 44.2
          - 33.8
          - 25.6
        * - TCP RX throughput (Mbit/s)
          - 63.6
          - 35.5
          - 42.3
          - 48.5
          - 40.5
          - 30.1
          - 27.8
        * - UDP TX throughput (Mbit/s)
          - 76.2
          - 75.1
          - 74.1
          - 72.4
          - 69.6
          - 64.1
          - 36.5
        * - UDP RX throughput (Mbit/s)
          - 83.1
          - 66.3
          - 75.1
          - 75.6
          - 73.1
          - 65.3
          - 54.7


.. only:: esp32s2

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 10 10 10

        * - Rank
          - Iperf
          - High-performance
          - Default
          - Memory saving
          - Minimum
        * - Available memory (KB)
          - 4.1
          - 24.2
          - 78.4
          - 86.5
          - 116.4
        * - WIFI_STATIC_RX_BUFFER_NUM
          - 8
          - 6
          - 6
          - 4
          - 3
        * - WIFI_DYNAMIC_RX_BUFFER_NUM
          - 24
          - 18
          - 12
          - 8
          - 6
        * - WIFI_DYNAMIC_TX_BUFFER_NUM
          - 24
          - 18
          - 12
          - 8
          - 6
        * - WIFI_RX_BA_WIN
          - 12
          - 9
          - 6
          - 4
          - 3
        * - TCP_SND_BUF_DEFAULT (KB)
          - 24
          - 18
          - 12
          - 8
          - 6
        * - TCP_WND_DEFAULT (KB)
          - 24
          - 18
          - 12
          - 8
          - 6
        * - WIFI_IRAM_OPT
          - ENABLE
          - ENABLE
          - ENABLE
          - ENABLE
          - DISABLE
        * - WIFI_RX_IRAM_OPT
          - ENABLE
          - ENABLE
          - ENABLE
          - DISABLE
          - DISABLE
        * - LWIP_IRAM_OPTIMIZATION
          - ENABLE
          - ENABLE
          - DISABLE
          - DISABLE
          - DISABLE
        * - INSTRUCTION_CACHE
          - 16
          - 16
          - 16
          - 16
          - 8
        * - INSTRUCTION_CACHE_LINE
          - 16
          - 16
          - 16
          - 16
          - 16
        * - TCP TX throughput (Mbit/s)
          - 37.6
          - 33.1
          - 22.5
          - 12.2
          - 5.5
        * - TCP RX throughput (Mbit/s)
          - 31.5
          - 28.1
          - 20.1
          - 13.1
          - 7.2
        * - UDP TX throughput (Mbit/s)
          - 58.1
          - 57.3
          - 28.1
          - 22.6
          - 8.7
        * - UDP RX throughput (Mbit/s)
          - 78.1
          - 66.7
          - 65.3
          - 53.8
          - 28.5

.. only:: esp32c3

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15

        * - Rank
          - Iperf
          - Default
          - Minimum
        * - Available memory (KB)
          - 59
          - 160
          - 180
        * - WIFI_STATIC_RX_BUFFER_NUM
          - 20
          - 8
          - 3
        * - WIFI_DYNAMIC_RX_BUFFER_NUM
          - 40
          - 16
          - 6
        * - WIFI_DYNAMIC_TX_BUFFER_NUM
          - 40
          - 16
          - 6
        * - WIFI_RX_BA_WIN
          - 32
          - 16
          - 6
        * - TCP_SND_BUF_DEFAULT (KB)
          - 40
          - 16
          - 6
        * - TCP_WND_DEFAULT (KB)
          - 40
          - 16
          - 6
        * - LWIP_IRAM_OPTIMIZATION
          - ENABLE
          - ENABLE
          - DISABLE
        * - TCP TX throughput (Mbit/s)
          - 38.1
          - 27.2
          - 20.4
        * - TCP RX throughput (Mbit/s)
          - 35.3
          - 24.2
          - 17.4
        * - UDP TX throughput (Mbit/s)
          - 40.6
          - 38.9
          - 34.1
        * - UDP RX throughput (Mbit/s)
          - 52.4
          - 44.5
          - 44.2

.. only:: esp32c6

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15

        * - Rank
          - Iperf
          - Default
          - Minimum
        * - Available memory (KB)
          - 223
          - 276
          - 299
        * - WIFI_STATIC_RX_BUFFER_NUM
          - 20
          - 8
          - 3
        * - WIFI_DYNAMIC_RX_BUFFER_NUM
          - 40
          - 16
          - 6
        * - WIFI_DYNAMIC_TX_BUFFER_NUM
          - 40
          - 16
          - 6
        * - WIFI_RX_BA_WIN
          - 32
          - 16
          - 6
        * - TCP_SND_BUF_DEFAULT (KB)
          - 40
          - 16
          - 6
        * - TCP_WND_DEFAULT (KB)
          - 40
          - 16
          - 6
        * - LWIP_IRAM_OPTIMIZATION
          - ENABLE
          - ENABLE
          - DISABLE
        * - TCP TX throughput (Mbit/s)
          - 30.5
          - 25.9
          - 16.4
        * - TCP RX throughput (Mbit/s)
          - 27.8
          - 21.6
          - 14.3
        * - UDP TX throughput (Mbit/s)
          - 37.8
          - 36.1
          - 34.6
        * - UDP RX throughput (Mbit/s)
          - 41.5
          - 36.8
          - 36.7

.. only:: esp32c2

     .. list-table::
        :header-rows: 1
        :widths: 10 10 10 15

        * - Rank
          - Iperf
          - Default
          - Minimum
        * - Available memory (KB)
          - 37
          - 56
          - 84
        * - WIFI_STATIC_RX_BUFFER_NUM
          - 14
          - 7
          - 3
        * - WIFI_DYNAMIC_RX_BUFFER_NUM
          - 18
          - 14
          - 6
        * - WIFI_DYNAMIC_TX_BUFFER_NUM
          - 18
          - 14
          - 6
        * - WIFI_RX_BA_WIN
          - 16
          - 12
          - 6
        * - TCP_SND_BUF_DEFAULT (KB)
          - 18
          - 14
          - 6
        * - TCP_WND_DEFAULT (KB)
          - 18
          - 14
          - 6
        * - LWIP_IRAM_OPTIMIZATION
          - ENABLE
          - ENABLE
          - DISABLE
        * - TCP TX throughput (Mbit/s)
          - 21.6
          - 21.4
          - 14.3
        * - TCP RX throughput (Mbit/s)
          - 19.1
          - 17.9
          - 12.4
        * - UDP TX throughput (Mbit/s)
          - 26.4
          - 26.3
          - 25.0
        * - UDP RX throughput (Mbit/s)
          - 32.3
          - 31.5
          - 27.7

.. only:: esp32s3

     .. list-table::
        :header-rows: 1
        :widths: 25 20 25 25

        * - Rank
          - Iperf
          - Default
          - Minimum
        * - Available memory (KB)
          - 133.9
          - 183.9
          - 273.6
        * - WIFI_STATIC_RX_BUFFER_NUM
          - 24
          - 8
          - 3
        * - WIFI_DYNAMIC_RX_BUFFER_NUM
          - 64
          - 32
          - 6
        * - WIFI_DYNAMIC_TX_BUFFER_NUM
          - 64
          - 32
          - 6
        * - WIFI_RX_BA_WIN
          - 32
          - 16
          - 6
        * - TCP_SND_BUF_DEFAULT (KB)
          - 64
          - 32
          - 6
        * - TCP_WND_DEFAULT (KB)
          - 64
          - 32
          - 6
        * - WIFI_IRAM_OPT
          - ENABLE
          - ENABLE
          - ENABLE
        * - WIFI_RX_IRAM_OPT
          - ENABLE
          - ENABLE
          - ENABLE
        * - LWIP_IRAM_OPTIMIZATION
          - ENABLE
          - ENABLE
          - DISABLE
        * - INSTRUCTION_CACHE
          - 32
          - 32
          - 16
        * - INSTRUCTION_CACHE_LINE
          - 32
          - 32
          - 32
        * - INSTRUCTION_CACHE_WAYS
          - 8
          - 8
          - 4
        * - TCP TX throughput (Mbit/s)
          - 83.93
          - 64.28
          - 23.17
        * - TCP RX throughput (Mbit/s)
          - 73.98
          - 60.39
          - 18.11
        * - UDP TX throughput (Mbit/s)
          - 98.69
          - 96.28
          - 48.78
        * - UDP RX throughput (Mbit/s)
          - 88.58
          - 86.57
          - 59.45

.. only:: esp32 or esp32s3

    .. note::
        The test was performed with a single stream in a shielded box using an ASUS RT-N66U router.
        {IDF_TARGET_NAME}'s CPU is dual core with 240 MHz. {IDF_TARGET_NAME}'s flash is in QIO mode with 80 MHz.

.. only:: esp32s2

    .. note::
        The test was performed with a single stream in a shielded box using an ASUS RT-N66U router.
        {IDF_TARGET_NAME}'s CPU is single core with 240 MHz. {IDF_TARGET_NAME}'s flash is in QIO mode with 80 MHz.

.. only:: esp32c3

    .. note::
        The test was performed with a single stream in a shielded box using an ASUS RT-N66U router.
        {IDF_TARGET_NAME}'s CPU is single core with 160 MHz. {IDF_TARGET_NAME}'s flash is in QIO mode with 80 MHz.

.. only:: esp32c6

    .. note::
        The test was performed with a single stream in a shielded box using an XIAOMI AX-6000 router.
        {IDF_TARGET_NAME}'s CPU is single core with 160 MHz. {IDF_TARGET_NAME}'s flash is in QIO mode with 80 MHz.

.. only:: esp32c2

    .. note::
        The test was performed with a single stream in a shielded box using an Redmi RM2100 router.
        {IDF_TARGET_NAME}'s CPU is single core with 120 MHz. {IDF_TARGET_NAME}'s flash is in QIO mode with 60 MHz.

.. only:: esp32

    **Ranks:**

     - **Iperf rank**
        {IDF_TARGET_NAME} extreme performance rank used to test extreme performance.

     - **High-performance rank**
        The {IDF_TARGET_NAME}'s high-performance configuration rank, suitable for scenarios where the application occupies less memory and has high-performance requirements. In this rank, users can choose to use the RX prior rank or the TX prior rank according to the usage scenario.

     - **Default rank**
        {IDF_TARGET_NAME}'s default configuration rank, the available memory, and performance are in balance.

     - **Memory saving rank**
        This rank is suitable for scenarios where the application requires a large amount of memory, and the transceiver performance will be reduced in this rank.

     - **Minimum rank**
        This is the minimum configuration rank of {IDF_TARGET_NAME}. The protocol stack only uses the necessary memory for running. It is suitable for scenarios where there is no requirement for performance and the application requires lots of space.

.. only:: esp32s2

    **Ranks:**

     - **Iperf rank**
        {IDF_TARGET_NAME} extreme performance rank used to test extreme performance.

     - **High-performance rank**
        The {IDF_TARGET_NAME}'s high-performance configuration rank, suitable for scenarios where the application occupies less memory and has high-performance requirements.

     - **Default rank**
        {IDF_TARGET_NAME}'s default configuration rank, the available memory, and performance are in balance.

     - **Memory saving rank**
        This rank is suitable for scenarios where the application requires a large amount of memory, and the transceiver performance will be reduced in this rank.

     - **Minimum rank**
        This is the minimum configuration rank of {IDF_TARGET_NAME}. The protocol stack only uses the necessary memory for running. It is suitable for scenarios where there is no requirement for performance and the application requires lots of space.

.. only:: esp32c3 or esp32s3 or esp32c6

    **Ranks:**

     - **Iperf rank**
        {IDF_TARGET_NAME} extreme performance rank used to test extreme performance.

     - **Default rank**
        {IDF_TARGET_NAME}'s default configuration rank, the available memory, and performance are in balance.

     - **Minimum rank**
        This is the minimum configuration rank of {IDF_TARGET_NAME}. The protocol stack only uses the necessary memory for running. It is suitable for scenarios where there is no requirement for performance and the application requires lots of space.

.. only:: SOC_SPIRAM_SUPPORTED

    Using PSRAM
    ++++++++++++++++++++++++++++

    PSRAM is generally used when the application takes up a lot of memory. In this mode, the :ref:`CONFIG_ESP_WIFI_TX_BUFFER` is forced to be static. :ref:`CONFIG_ESP_WIFI_STATIC_TX_BUFFER_NUM` indicates the number of DMA buffers at the hardware layer, and increasing this parameter can improve performance.
    The following are the recommended ranks for using PSRAM:

    .. only:: esp32

        .. list-table::
             :header-rows: 1
             :widths: 15 10 10 15 10

             * - Rank
               - Iperf
               - Default
               - Memory saving
               - Minimum
             * - Available memory (KB)
               - 113.8
               - 152.4
               - 181.2
               - 202.6
             * - WIFI_STATIC_RX_BUFFER_NUM
               - 16
               - 8
               - 4
               - 2
             * - WIFI_DYNAMIC_RX_BUFFER_NUM
               - 128
               - 128
               - 128
               - 128
             * - WIFI_STATIC_TX_BUFFER_NUM
               - 16
               - 8
               - 4
               - 2
             * - WIFI_RX_BA_WIN
               - 16
               - 16
               - 8
               - Disable
             * - TCP_SND_BUF_DEFAULT (KB)
               - 65
               - 65
               - 65
               - 65
             * - TCP_WND_DEFAULT (KB)
               - 65
               - 65
               - 65
               - 65
             * - WIFI_IRAM_OPT
               - ENABLE
               - ENABLE
               - ENABLE
               - DISABLE
             * - WIFI_RX_IRAM_OPT
               - ENABLE
               - ENABLE
               - DISABLE
               - DISABLE
             * - LWIP_IRAM_OPTIMIZATION
               - ENABLE
               - DISABLE
               - DISABLE
               - DISABLE
             * - TCP TX throughput (Mbit/s)
               - 37.5
               - 31.7
               - 21.7
               - 14.6
             * - TCP RX throughput (Mbit/s)
               - 31.5
               - 29.8
               - 26.5
               - 21.1
             * - UDP TX throughput (Mbit/s)
               - 69.1
               - 31.5
               - 27.1
               - 24.1
             * - UDP RX throughput (Mbit/s)
               - 40.1
               - 38.5
               - 37.5
               - 36.9

    .. only:: esp32s2

        .. list-table::
             :header-rows: 1
             :widths: 10 10 10 10 15

             * - Rank
               - Iperf
               - Default
               - Memory saving
               - Minimum
             * - Available memory (KB)
               - 70.6
               - 96.4
               - 118.8
               - 148.2
             * - WIFI_STATIC_RX_BUFFER_NUM
               - 8
               - 8
               - 6
               - 4
             * - WIFI_DYNAMIC_RX_BUFFER_NUM
               - 64
               - 64
               - 64
               - 64
             * - WIFI_STATIC_TX_BUFFER_NUM
               - 16
               - 8
               - 6
               - 4
             * - WIFI_RX_BA_WIN
               - 16
               - 6
               - 6
               - Disable
             * - TCP_SND_BUF_DEFAULT (KB)
               - 32
               - 32
               - 32
               - 32
             * - TCP_WND_DEFAULT (KB)
               - 32
               - 32
               - 32
               - 32
             * - WIFI_IRAM_OPT
               - ENABLE
               - ENABLE
               - ENABLE
               - DISABLE
             * - WIFI_RX_IRAM_OPT
               - ENABLE
               - ENABLE
               - DISABLE
               - DISABLE
             * - LWIP_IRAM_OPTIMIZATION
               - ENABLE
               - DISABLE
               - DISABLE
               - DISABLE
             * - INSTRUCTION_CACHE
               - 16
               - 16
               - 16
               - 8
             * - INSTRUCTION_CACHE_LINE
               - 16
               - 16
               - 16
               - 16
             * - DATA_CACHE
               - 8
               - 8
               - 8
               - 8
             * - DATA_CACHE_LINE
               - 32
               - 32
               - 32
               - 32
             * - TCP TX throughput (Mbit/s)
               - 40.1
               - 29.2
               - 20.1
               - 8.9
             * - TCP RX throughput (Mbit/s)
               - 21.9
               - 16.8
               - 14.8
               - 9.6
             * - UDP TX throughput (Mbit/s)
               - 50.1
               - 25.7
               - 22.4
               - 10.2
             * - UDP RX throughput (Mbit/s)
               - 45.3
               - 43.1
               - 28.5
               - 15.1

        .. note::
            Reaching peak performance may cause task watchdog. It is a normal phenomenon considering the CPU may have no time for lower priority tasks.

    .. only:: esp32s3

        **PSRAM with 4 lines:**

          .. list-table::
             :header-rows: 1
             :widths: 25 20 25 25 25

             * - Rank
               - Iperf
               - Default
               - Memory saving
               - Minimum
             * - Available memory (KB)
               - 50.3
               - 158.7
               - 198.2
               - 228.9
             * - WIFI_STATIC_RX_BUFFER_NUM
               - 24
               - 8
               - 6
               - 4
             * - WIFI_DYNAMIC_RX_BUFFER_NUM
               - 85
               - 64
               - 32
               - 32
             * - WIFI_STATIC_TX_BUFFER_NUM
               - 32
               - 32
               - 6
               - 4
             * - WIFI_RX_BA_WIN
               - 32
               - 16
               - 12
               - Disable
             * - TCP_SND_BUF_DEFAULT (KB)
               - 85
               - 32
               - 32
               - 32
             * - TCP_WND_DEFAULT (KB)
               - 85
               - 32
               - 32
               - 32
             * - WIFI_IRAM_OPT
               - ENABLE
               - ENABLE
               - ENABLE
               - DISABLE
             * - WIFI_RX_IRAM_OPT
               - ENABLE
               - ENABLE
               - DISABLE
               - DISABLE
             * - LWIP_IRAM_OPTIMIZATION
               - ENABLE
               - DISABLE
               - DISABLE
               - DISABLE
             * - LWIP_UDP_RECVMBOX_SIZE
               - 16
               - 16
               - 16
               - 16
             * - INSTRUCTION_CACHE
               - 32
               - 16
               - 16
               - 16
             * - INSTRUCTION_CACHE_LINE
               - 32
               - 16
               - 16
               - 16
             * - INSTRUCTION_CACHE_WAYS
               - 8
               - 8
               - 8
               - 8
             * - DATA_CACHE
               - 64
               - 16
               - 16
               - 16
             * - DATA_CACHE_LINE
               - 32
               - 32
               - 32
               - 32
             * - DATA_CACHE_WAYS
               - 8
               - 8
               - 8
               - 8
             * - TCP TX throughput (Mbit/s)
               - 93.1
               - 62.5
               - 41.3
               - 42.7
             * - TCP RX throughput (Mbit/s)
               - 88.9
               - 46.5
               - 46.2
               - 37.9
             * - UDP TX throughput (Mbit/s)
               - 106.4
               - 106.2
               - 60.7
               - 50
             * - UDP RX throughput (Mbit/s)
               - 99.8
               - 92.6
               - 94.3
               - 53.3

        **PSRAM with 8 lines:**

          .. list-table::
             :header-rows: 1
             :widths: 25 20 25 25 25

             * - Rank
               - Iperf
               - Default
               - Memory saving
               - Minimum
             * - Available memory (KB)
               - 49.1
               - 151.3
               - 215.3
               - 243.6
             * - WIFI_STATIC_RX_BUFFER_NUM
               - 24
               - 8
               - 6
               - 4
             * - WIFI_DYNAMIC_RX_BUFFER_NUM
               - 85
               - 64
               - 32
               - 32
             * - WIFI_STATIC_TX_BUFFER_NUM
               - 32
               - 32
               - 6
               - 4
             * - WIFI_RX_BA_WIN
               - 32
               - 16
               - 12
               - Disable
             * - TCP_SND_BUF_DEFAULT (KB)
               - 85
               - 32
               - 32
               - 32
             * - TCP_WND_DEFAULT (KB)
               - 85
               - 32
               - 32
               - 32
             * - WIFI_IRAM_OPT
               - ENABLE
               - ENABLE
               - ENABLE
               - DISABLE
             * - WIFI_RX_IRAM_OPT
               - ENABLE
               - ENABLE
               - DISABLE
               - DISABLE
             * - LWIP_IRAM_OPTIMIZATION
               - ENABLE
               - DISABLE
               - DISABLE
               - DISABLE
             * - LWIP_UDP_RECVMBOX_SIZE
               - 16
               - 16
               - 16
               - 16
             * - INSTRUCTION_CACHE
               - 32
               - 16
               - 16
               - 16
             * - INSTRUCTION_CACHE_LINE
               - 32
               - 16
               - 16
               - 16
             * - INSTRUCTION_CACHE_WAYS
               - 8
               - 8
               - 8
               - 8
             * - DATA_CACHE
               - 64
               - 16
               - 16
               - 16
             * - DATA_CACHE_LINE
               - 32
               - 32
               - 32
               - 32
             * - DATA_CACHE_WAYS
               - 8
               - 8
               - 8
               - 8
             * - TCP TX throughput (Mbit/s)
               - 93.3
               - 58.4
               - 37.1
               - 35.6
             * - TCP RX throughput (Mbit/s)
               - 86.1
               - 43.6
               - 42.5
               - 35
             * - UDP TX throughput (Mbit/s)
               - 104.7
               - 82.2
               - 60.4
               - 47.9
             * - UDP RX throughput (Mbit/s)
               - 104.6
               - 104.8
               - 104
               - 55.7


Wi-Fi Menuconfig
-----------------------

Wi-Fi Buffer Configure
+++++++++++++++++++++++

If you are going to modify the default number or type of buffer, it would be helpful to also have an overview of how the buffer is allocated/freed in the data path. The following diagram shows this process in the TX direction:

.. blockdiag::
    :caption: TX Buffer Allocation
    :align: center

    blockdiag buffer_allocation_tx {

        # global attributes
        node_height = 60;
        node_width = 100;
        span_width = 50;
        span_height = 20;
        default_shape = roundedbox;

        # labels of diagram nodes
        APPL_TASK [label="Application\n task", fontsize=12];
        LwIP_TASK [label="LwIP\n task", fontsize=12];
        WIFI_TASK [label="Wi-Fi\n task", fontsize=12];

        # labels of description nodes
        APPL_DESC [label="1> User data", width=120, height=25, shape=note, color=yellow];
        LwIP_DESC [label="2> Pbuf", width=120, height=25, shape=note, color=yellow];
        WIFI_DESC [label="3> Dynamic (Static)\n TX Buffer", width=150, height=40, shape=note, color=yellow];

        # node connections
        APPL_TASK -> LwIP_TASK -> WIFI_TASK
        APPL_DESC -> LwIP_DESC -> WIFI_DESC [style=none]
    }


Description:

 - The application allocates the data which needs to be sent out.
 - The application calls TCPIP-/Socket-related APIs to send the user data. These APIs will allocate a PBUF used in LwIP, and make a copy of the user data.
 - When LwIP calls a Wi-Fi API to send the PBUF, the Wi-Fi API will allocate a "Dynamic Tx Buffer" or "Static Tx Buffer", make a copy of the LwIP PBUF, and finally send the data.

The following diagram shows how buffer is allocated/freed in the RX direction:

.. blockdiag::
    :caption: RX Buffer Allocation
    :align: center

    blockdiag buffer_allocation_rx {

        # global attributes
        node_height = 60;
        node_width = 100;
        span_width = 40;
        span_height = 20;
        default_shape = roundedbox;

        # labels of diagram nodes
        APPL_TASK [label="Application\n task", fontsize=12];
        LwIP_TASK [label="LwIP\n task", fontsize=12];
        WIFI_TASK [label="Wi-Fi\n task", fontsize=12];
        WIFI_INTR [label="Wi-Fi\n interrupt", fontsize=12];

        # labels of description nodes
        APPL_DESC [label="4> User\n Data Buffer", height=40, shape=note, color=yellow];
        LwIP_DESC [label="3> Pbuf", height=40, shape=note, color=yellow];
        WIFI_DESC [label="2> Dynamic\n RX Buffer", height=40, shape=note, color=yellow];
        INTR_DESC [label="1> Static\n RX Buffer", height=40, shape=note, color=yellow];

        # node connections
        APPL_TASK <- LwIP_TASK <- WIFI_TASK <- WIFI_INTR
        APPL_DESC <- LwIP_DESC <- WIFI_DESC <- INTR_DESC [style=none]
    }

Description:

 - The Wi-Fi hardware receives a packet over the air and puts the packet content to the "Static Rx Buffer", which is also called "RX DMA Buffer".
 - The Wi-Fi driver allocates a "Dynamic Rx Buffer", makes a copy of the "Static Rx Buffer", and returns the "Static Rx Buffer" to hardware.
 - The Wi-Fi driver delivers the packet to the upper-layer (LwIP), and allocates a PBUF for holding the "Dynamic Rx Buffer".
 - The application receives data from LwIP.

The diagram shows the configuration of the Wi-Fi internal buffer.

.. list-table::
   :header-rows: 1
   :widths: 10 10 10 10 25

   * - Buffer Type
     - Alloc Type
     - Default
     - Configurable
     - Description
   * - Static RX Buffer (Hardware RX Buffer)
     - Static
     - 10 * 1600 Bytes
     - Yes
     - This is a kind of DMA memory. It is initialized in :cpp:func:`esp_wifi_init()` and freed in :cpp:func:`esp_wifi_deinit()`. The ‘Static Rx Buffer’ forms the hardware receiving list. Upon receiving a frame over the air, hardware writes the frame into the buffer and raises an interrupt to the CPU. Then, the Wi-Fi driver reads the content from the buffer and returns the buffer back to the list.

       If needs be, the application can reduce the memory statically allocated by Wi-Fi. It can reduce this value from 10 to 6 to save 6400 Bytes of memory. It is not recommended to reduce the configuration to a value less than 6 unless the AMPDU feature is disabled.
   * - Dynamic RX Buffer
     - Dynamic
     - 32
     - Yes
     - The buffer length is variable and it depends on the received frames’ length. When the Wi-Fi driver receives a frame from the ‘Hardware Rx Buffer’, the ‘Dynamic Rx Buffer’ needs to be allocated from the heap. The number of the Dynamic Rx Buffer, configured in the menuconfig, is used to limit the total un-freed Dynamic Rx Buffer number.
   * - Dynamic TX Buffer
     - Dynamic
     - 32
     - Yes
     - This is a kind of DMA memory. It is allocated to the heap. When the upper-layer (LwIP) sends packets to the Wi-Fi driver, it firstly allocates a ‘Dynamic TX Buffer’ and makes a copy of the upper-layer buffer.

       The Dynamic and Static TX Buffers are mutually exclusive.
   * - Static TX Buffer
     - Static
     - 16 * 1600Bytes
     - Yes
     - This is a kind of DMA memory. It is initialized in :cpp:func:`esp_wifi_init()` and freed in :cpp:func:`esp_wifi_deinit()`. When the upper-layer (LwIP) sends packets to the Wi-Fi driver, it firstly allocates a ‘Static TX Buffer’ and makes a copy of the upper-layer buffer.

       The Dynamic and Static TX Buffer are mutually exclusive.

       The TX buffer must be a DMA buffer. For this reason, if PSRAM is enabled, the TX buffer must be static.
   * - Management Short Buffer
     - Dynamic
     - 8
     - NO
     - Wi-Fi driver’s internal buffer.
   * - Management Long Buffer
     - Dynamic
     - 32
     - NO
     - Wi-Fi driver’s internal buffer.
   * - Management Long Long Buffer
     - Dynamic
     - 32
     - NO
     - Wi-Fi driver’s internal buffer.

Wi-Fi NVS Flash
+++++++++++++++++++++

If the Wi-Fi NVS flash is enabled, all Wi-Fi configurations set via the Wi-Fi APIs will be stored into flash, and the Wi-Fi driver will start up with these configurations the next time it powers on/reboots. However, the application can choose to disable the Wi-Fi NVS flash if it does not need to store the configurations into persistent memory, or has its own persistent storage, or simply due to debugging reasons, etc.

Wi-Fi Aggregate MAC Protocol Data Unit (AMPDU)
++++++++++++++++++++++++++++++++++++++++++++++++++++++

{IDF_TARGET_NAME} supports both receiving and transmitting AMPDU, and the AMPDU can greatly improve the Wi-Fi throughput.

Generally, the AMPDU should be enabled. Disabling AMPDU is usually for debugging purposes.

Troubleshooting
---------------

Please refer to a separate document with :doc:`wireshark-user-guide`.

.. toctree::
    :hidden:

    wireshark-user-guide
````

## File: docs/en/api-guides/wireshark-user-guide.rst
````
******************************
Espressif Wireshark User Guide
******************************

:link_to_translation:`zh_CN:[中文]`

===========
1. Overview
===========

1.1 What Is Wireshark?
======================

`Wireshark <https://www.wireshark.org>`_ (originally named "Ethereal") is a network packet analyzer that captures network packets and displays the packet data as detailed as possible. It uses WinPcap as its interface to directly capture network traffic going through a network interface controller (NIC).

You could think of a network packet analyzer as a measuring device used to examine what is going on inside a network cable, just like a voltmeter is used by an electrician to examine what is going on inside an electric cable.

In the past, such tools were either very expensive, proprietary, or both. However, with the advent of Wireshark, all that has changed.

Wireshark is released under the terms of the GNU General Public License, which means you can use the software and the source code free of charge. It also allows you to modify and customize the source code.

Wireshark is, perhaps, one of the best open source packet analyzers available today.


1.2 Some Intended Purposes
==========================

Here are some examples of how Wireshark is typically used:

* Network administrators use it to troubleshoot network problems.

* Network security engineers use it to examine security problems.

* Developers use it to debug protocol implementations.

* People use it to learn more about network protocol internals.

Beside these examples, Wireshark can be used for many other purposes.


1.3 Features
============

The main features of Wireshark are as follows:

* Available for UNIX and Windows

* Captures live packet data from a network interface

* Displays packets along with detailed protocol information

* Opens/saves the captured packet data

* Imports/exports packets into a number of file formats, supported by other capture programs

* Advanced packet filtering

* Searches for packets based on multiple criteria

* Colorizes packets according to display filters

* Calculates statistics

* ... and a lot more!


1.4 Wireshark Can or Cannot Do
==============================

* **Live capture from different network media**.

  Wireshark can capture traffic from different network media, including wireless LAN.

* **Import files from many other capture programs**.

  Wireshark can import data from a large number of file formats, supported by other capture programs.

* **Export files for many other capture programs**.

  Wireshark can export data into a large number of file formats, supported by other capture programs.

* **Numerous protocol dissectors**.

  Wireshark can dissect, or decode, a large number of protocols.

* **Wireshark is not an intrusion detection system**.

  It will not warn you if there are any suspicious activities on your network. However, if strange things happen, Wireshark might help you figure out what is really going on.

* **Wireshark does not manipulate processes on the network, it can only perform "measurements" within it**.

  Wireshark does not send packets on the network or influence it in any other way, except for resolving names (converting numerical address values into a human readable format), but even that can be disabled.


==========================
1. Where to Get Wireshark
==========================

You can get Wireshark from the official website: https://www.wireshark.org/download.html

Wireshark can run on various operating systems. Please download the correct version according to the operating system you are using.


======================
3. Step-by-step Guide
======================

**This demonstration uses Wireshark 2.2.6 on Linux.**


**a) Start Wireshark**

On Linux, you can run the shell script provided below. It starts Wireshark, then configures NIC and the channel for packet capture.

::

  ifconfig $1 down
  iwconfig $1 mode monitor
  iwconfig $1 channel $2
  ifconfig $1 up
  Wireshark&

In the above script, the parameter ``$1`` represents NIC and ``$2`` represents channel. For example, ``wlan0`` in ``./xxx.sh wlan0 6``, specifies the NIC for packet capture, and ``6`` identifies the channel of an AP or Soft-AP.


**b) Run the Shell Script to Open Wireshark and Display Capture Interface**

.. figure:: ../../_static/ws-capture-interface.jpeg
    :align: center
    :alt: Wireshark Capture Interface
    :figclass: align-center
    :width: 60%

    Wireshark Capture Interface

**c) Select the Interface to Start Packet Capture**

As the red markup shows in the picture above, many interfaces are available. The first one is a local NIC and the second one is a wireless NIC.

Please select the NIC according to your requirements. This document will use the wireless NIC to demonstrate packet capture.

Double click *wlan0* to start packet capture.


**d) Set up Filters**

Since all packets in the channel will be captured, and many of them are not needed, you have to set up filters to get the packets that you need.

Please find the picture below with the red markup, indicating where the filters should be set up.

.. figure:: ../../_static/ws-setup-filters.png
    :align: center
    :alt: Setting up Filters in Wireshark
    :figclass: align-center

    Setting up Filters in Wireshark

Click *Filter*, the top left blue button in the picture below. The *display filter* dialogue box will appear.

.. figure:: ../../_static/ws-display-filter-dialogue-box.png
    :align: center
    :alt: *Display Filter* Dialogue Box
    :figclass: align-center
    :width: 60%

    *Display Filter* Dialogue Box

Click the *Expression* button to bring up the *Filter Expression* dialogue box and set the filter according to your requirements.

.. figure:: ../../_static/ws-filter-expression.png
    :align: center
    :alt: *Filter Expression* Dialogue Box
    :figclass: align-center
    :width: 80%

    *Filter Expression* Dialogue Box

**The quickest way**: enter the filters directly in the toolbar.

.. figure:: ../../_static/ws-filter-toolbar.png
    :align: center
    :alt: Filter Toolbar
    :figclass: align-center

    Filter Toolbar

Click on this area to enter or modify the filters. If you enter a wrong or unfinished filter, the built-in syntax check turns the background red. As soon as the correct expression is entered, the background becomes green.

The previously entered filters are automatically saved. You can access them anytime by opening the drop down list.

For example, as shown in the picture below, enter two MAC addresses as the filters and click *Apply* (the blue arrow). In this case, only the packet data transmitted between these two MAC addresses will be captured.

.. figure:: ../../_static/ws-filter-toolbar_green.png
    :align: center
    :alt: Example of MAC Addresses applied in the Filter Toolbar
    :figclass: align-center

    Example of MAC Addresses applied in the Filter Toolbar

**e) Packet List**

You can click any packet in the packet list and check the detailed information about it in the box below the list. For example, if you click the first packet, its details will appear in that box.

.. figure:: ../../_static/ws-packet-list.png
    :align: center
    :alt: Example of Packet List Details
    :figclass: align-center

    Example of Packet List Details

**f) Stop/Start Packet Capture**

As shown in the picture below, click the red button to stop capturing the current packet.

.. figure:: ../../_static/ws-stop-packet-capture.png
    :align: center
    :alt: Stopping Packets Capture
    :figclass: align-center

    Stopping Packet Capture

Click the top left blue button to start or resume packet capture.

.. figure:: ../../_static/ws-start-resume-packet-capture.png
    :align: center
    :alt: Starting or Resuming Packets Capture
    :figclass: align-center
    :width: 60%

    Starting or Resuming the Packets Capture

**g) Save the Current Packet**

On Linux, go to *File* -> *Export Packet Dissections* -> *As Plain Text File* to save the packet.

.. figure:: ../../_static/ws-save-packets.png
    :align: center
    :alt: Saving Captured Packets
    :figclass: align-center
    :width: 60%

    Saving Captured Packets

Please note that *All packets*, *Displayed* and *All expanded* must be selected.

By default, Wireshark saves the captured packet in a libpcap file. You can also save the file in other formats, e.g., txt, to analyze it in other tools.
````
