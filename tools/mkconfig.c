/****************************************************************************
 * tools/mkconfig.c
 *
 *   Copyright (C) 2007-2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "cfgdefine.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DEFCONFIG ".config"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline char *getfilepath(const char *name)
{
  snprintf(line, PATH_MAX, "%s/" DEFCONFIG, name);
  line[PATH_MAX] = '\0';
  return strdup(line);
}

static void show_usage(const char *progname)
{
  fprintf(stderr, "USAGE: %s <abs path to .config>\n", progname);
  exit(1);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char **argv, char **envp)
{
  char *filepath;
  FILE *stream;

  if (argc != 2)
    {
      fprintf(stderr, "Unexpected number of arguments\n");
      show_usage(argv[0]);
    }

  filepath = getfilepath(argv[1]);
  if (!filepath)
    {
      fprintf(stderr, "getfilepath failed\n");
      exit(2);
    }

  stream = fopen(filepath, "r");
  if (!stream)
    {
      fprintf(stderr, "open %s failed: %s\n", filepath, strerror(errno));
      exit(3);
    }

  printf("/* config.h -- Autogenerated! Do not edit. */\n\n");
  printf("#ifndef __INCLUDE_NUTTX_CONFIG_H\n");
  printf("#define __INCLUDE_NUTTX_CONFIG_H\n\n");
  printf("/* Architecture-specific options *************************/\n\n");

  generate_definitions(stream);

  printf("\n/* Sanity Checks *****************************************/\n\n");
  printf("/* If this is an NXFLAT, external build, then make sure that\n");
  printf(" * NXFLAT support is enabled in the base code.\n");
  printf(" */\n\n");
  printf("#if defined(__NXFLAT__) && !defined(CONFIG_NXFLAT)\n");
  printf("# error \"NXFLAT support not enabled in this configuration\"\n");
  printf("#endif\n\n");
  printf("/* NXFLAT requires PIC support in the TCBs. */\n\n");
  printf("#if defined(CONFIG_NXFLAT)\n");
  printf("# undef CONFIG_PIC\n");
  printf("# define CONFIG_PIC 1\n");
  printf("#endif\n\n");
  printf("/* Binary format support is disabled if no binary formats are\n");
  printf(" * configured (at present, NXFLAT is the only supported binary.\n");
  printf(" * format).\n");
  printf(" */\n\n");
  printf("#if !defined(CONFIG_NXFLAT) && !defined(CONFIG_ELF) && !defined(CONFIG_BUILTIN)\n");
  printf("# undef CONFIG_BINFMT_DISABLE\n");
  printf("# define CONFIG_BINFMT_DISABLE 1\n");
  printf("#endif\n\n");
  printf("/* The correct way to disable RR scheduling is to set the\n");
  printf(" * timeslice to zero.\n");
  printf(" */\n\n");
  printf("#ifndef CONFIG_RR_INTERVAL\n");
  printf("# define CONFIG_RR_INTERVAL 0\n");
  printf("#endif\n\n");
  printf("/* The correct way to disable filesystem supuport is to set the number of\n");
  printf(" * file descriptors to zero.\n");
  printf(" */\n\n");
  printf("#ifndef CONFIG_NFILE_DESCRIPTORS\n");
  printf("# define CONFIG_NFILE_DESCRIPTORS 0\n");
  printf("#endif\n\n");
  printf("/* If a console is selected, then make sure that there are resources for\n");
  printf(" * three file descriptors and, if any streams are selected, also for three\n");
  printf(" * file streams.\n");
  printf(" *\n");
  printf(" * CONFIG_DEV_CONSOLE means that a builtin console device exists at /dev/console\n");
  printf(" * and can be opened during boot-up.  Other consoles, such as USB consoles, may\n");
  printf(" * not exist at boot-upand have to be handled in a different way.  Three file\n");
  printf(" * descriptors and three file streams are still needed.\n");
  printf(" */\n\n");
  printf("#if defined(CONFIG_DEV_CONSOLE) || defined(CONFIG_CDCACM_CONSOLE) || \\\n");
  printf("    defined(CONFIG_PL2303_CONSOLE)\n");
  printf("# if CONFIG_NFILE_DESCRIPTORS < 3\n");
  printf("#   undef CONFIG_NFILE_DESCRIPTORS\n");
  printf("#   define CONFIG_NFILE_DESCRIPTORS 3\n");
  printf("# endif\n\n");
  printf("# if CONFIG_NFILE_STREAMS > 0 && CONFIG_NFILE_STREAMS < 3\n");
  printf("#  undef CONFIG_NFILE_STREAMS\n");
  printf("#  define CONFIG_NFILE_STREAMS 3\n");
  printf("# endif\n\n");
  printf("/* If no console is selected, then disable all builtin console devices */\n\n");
  printf("#else\n");
  printf("#  undef CONFIG_DEV_LOWCONSOLE\n");
  printf("#  undef CONFIG_RAMLOG_CONSOLE\n");
  printf("#endif\n\n");
  printf("/* If priority inheritance is disabled, then do not allocate any\n");
  printf(" * associated resources.\n");
  printf(" */\n\n");
  printf("#if !defined(CONFIG_PRIORITY_INHERITANCE) || !defined(CONFIG_SEM_PREALLOCHOLDERS)\n");
  printf("# undef CONFIG_SEM_PREALLOCHOLDERS\n");
  printf("# define CONFIG_SEM_PREALLOCHOLDERS 0\n");
  printf("#endif\n\n");
  printf("#if !defined(CONFIG_PRIORITY_INHERITANCE) || !defined(CONFIG_SEM_NNESTPRIO)\n");
  printf("# undef CONFIG_SEM_NNESTPRIO\n");
  printf("# define CONFIG_SEM_NNESTPRIO 0\n");
  printf("#endif\n\n");
  printf("/* If no file descriptors are configured, then make certain no\n");
  printf(" * streams are configured either.\n");
  printf(" */\n\n");
  printf("#if CONFIG_NFILE_DESCRIPTORS == 0\n");
  printf("# undef CONFIG_NFILE_STREAMS\n");
  printf("# define CONFIG_NFILE_STREAMS 0\n");
  printf("#endif\n\n");
  printf("/* There must be at least one memory region. */\n\n");
  printf("#ifndef CONFIG_MM_REGIONS\n");
  printf("# define CONFIG_MM_REGIONS 1\n");
  printf("#endif\n\n");
  printf("/* If the end of RAM is not specified then it is assumed to be the beginning\n");
  printf(" * of RAM plus the RAM size.\n");
  printf(" */\n\n");
  printf("#ifndef CONFIG_RAM_END\n");
  printf("# define CONFIG_RAM_END (CONFIG_RAM_START+CONFIG_RAM_SIZE)\n");
  printf("#endif\n\n");
  printf("#ifndef CONFIG_RAM_VEND\n");
  printf("# define CONFIG_RAM_VEND (CONFIG_RAM_VSTART+CONFIG_RAM_SIZE)\n");
  printf("#endif\n\n");
  printf("/* If the end of FLASH is not specified then it is assumed to be the beginning\n");
  printf(" * of FLASH plus the FLASH size.\n");
  printf(" */\n\n");
  printf("#ifndef CONFIG_FLASH_END\n");
  printf("# define CONFIG_FLASH_END (CONFIG_FLASH_START+CONFIG_FLASH_SIZE)\n");
  printf("#endif\n\n");
  printf("/* If no file streams are configured, then make certain that buffered I/O\n");
  printf(" * support is disabled\n");
  printf(" */\n\n");
  printf("#if CONFIG_NFILE_STREAMS == 0\n");
  printf("# undef CONFIG_STDIO_BUFFER_SIZE\n");
  printf("# define CONFIG_STDIO_BUFFER_SIZE 0\n");
  printf("#endif\n\n");
  printf("/* If no standard C buffered I/O is not supported, then line-oriented buffering\n");
  printf(" * cannot be supported.\n");
  printf(" */\n\n");
  printf("#if CONFIG_STDIO_BUFFER_SIZE == 0\n");
  printf("# undef CONFIG_STDIO_LINEBUFFER\n");
  printf("#endif\n\n");
  printf("/* If the maximum message size is zero, then we assume that message queues\n");
  printf(" * support should be disabled\n");
  printf(" */\n\n");
  printf("#if !defined(CONFIG_MQ_MAXMSGSIZE) || defined(CONFIG_DISABLE_MQUEUE)\n");
  printf("# undef CONFIG_MQ_MAXMSGSIZE\n");
  printf("# define CONFIG_MQ_MAXMSGSIZE 0\n");
  printf("#endif\n\n");
  printf("#if CONFIG_MQ_MAXMSGSIZE <= 0 && !defined(CONFIG_DISABLE_MQUEUE)\n");
  printf("# define CONFIG_DISABLE_MQUEUE 1\n");
  printf("#endif\n\n");
  printf("/* If mountpoint support in not included, then no filesystem can be supported */\n\n");
  printf("#ifdef CONFIG_DISABLE_MOUNTPOINT\n");
  printf("# undef CONFIG_FS_FAT\n");
  printf("# undef CONFIG_FS_ROMFS\n");
  printf("# undef CONFIG_FS_NXFFS\n");
  printf("# undef CONFIG_FS_SMARTFS\n");
  printf("# undef CONFIG_FS_BINFS\n");
  printf("# undef CONFIG_NFS\n");
  printf("#endif\n\n");
  printf("/* There can be no network support with no socket descriptors */\n\n");
  printf("#if CONFIG_NSOCKET_DESCRIPTORS <= 0\n");
  printf("# undef CONFIG_NET\n");
  printf("#endif\n\n");
  printf("/* Conversely, if there is no network support, there is no need for\n");
  printf(" * socket descriptors\n");
  printf(" */\n\n");
  printf("#ifndef CONFIG_NET\n");
  printf("# undef CONFIG_NSOCKET_DESCRIPTORS\n");
  printf("# define CONFIG_NSOCKET_DESCRIPTORS 0\n");
  printf("#endif\n\n");
  printf("/* Protocol support can only be provided on top of basic network support */\n\n");
  printf("#ifndef CONFIG_NET\n");
  printf("# undef CONFIG_NET_TCP\n");
  printf("# undef CONFIG_NET_UDP\n");
  printf("# undef CONFIG_NET_ICMP\n");
  printf("#endif\n\n");
  printf("/* NFS client can only be provided on top of UDP network support */\n\n");
  printf("#if !defined(CONFIG_NET) || !defined(CONFIG_NET_UDP)\n");
  printf("# undef CONFIG_NFS\n");
  printf("#endif\n\n");
  printf("/* Verbose debug and sub-system debug only make sense if debug is enabled */\n\n");
  printf("#ifndef CONFIG_DEBUG\n");
  printf("# undef CONFIG_DEBUG_VERBOSE\n");
  printf("# undef CONFIG_DEBUG_SCHED\n");
  printf("# undef CONFIG_DEBUG_MM\n");
  printf("# undef CONFIG_DEBUG_PAGING\n");
  printf("# undef CONFIG_DEBUG_DMA\n");
  printf("# undef CONFIG_DEBUG_FS\n");
  printf("# undef CONFIG_DEBUG_LIB\n");
  printf("# undef CONFIG_DEBUG_BINFMT\n");
  printf("# undef CONFIG_DEBUG_NET\n");
  printf("# undef CONFIG_DEBUG_USB\n");
  printf("# undef CONFIG_DEBUG_GRAPHICS\n");
  printf("# undef CONFIG_DEBUG_GPIO\n");
  printf("# undef CONFIG_DEBUG_SPI\n");
  printf("# undef CONFIG_DEBUG_HEAP\n");
  printf("#endif\n\n");
  printf("/* User entry point. This is provided as a fall-back to keep compatibility\n");
  printf(" * with existing code, for builds which do not define CONFIG_USER_ENTRYPOINT.\n");
  printf(" */\n\n");
  printf("#ifndef CONFIG_USER_ENTRYPOINT\n");
  printf("# define CONFIG_USER_ENTRYPOINT main\n");
  printf("#endif\n\n");
  printf("#endif /* __INCLUDE_NUTTX_CONFIG_H */\n");
  fclose(stream);

  /* Exit (without bothering to clean up allocations) */

  free(filepath);
  return 0;
}
