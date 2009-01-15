/*  FileWriter-Plugin
 *  (C) copyright 2007 merging of Disk Writer and Out-Lame by Michael Färber
 *
 *  Original Out-Lame-Plugin:
 *  (C) copyright 2002 Lars Siebold <khandha5@gmx.net>
 *  (C) copyright 2006-2007 porting to audacious by Yoshiki Yazawa <yaz@cc.rim.or.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "plugins.h"

static gint wav_open(void);
static void wav_write(void *ptr, gint length);
static void wav_flush(void);
static void wav_close(void);
static gint wav_free(void);
static gint wav_playing(void);
static gint wav_get_written_time(void);

FileWriter wav_plugin =
{
    NULL,
    NULL,
    wav_open,
    wav_write,
    wav_flush,
    wav_close,
    wav_free,
    wav_playing,
    wav_get_written_time,
    FMT_S16_LE
};


struct wavhead
{
    guint32 main_chunk;
    guint32 length;
    guint32 chunk_type;
    guint32 sub_chunk;
    guint32 sc_len;
    guint16 format;
    guint16 modus;
    guint32 sample_fq;
    guint32 byte_p_sec;
    guint16 byte_p_spl;
    guint16 bit_p_spl;
    guint32 data_chunk;
    guint32 data_length;
};
static struct wavhead header;

static guint64 written;

static gint wav_open(void)
{
    memcpy(&header.main_chunk, "RIFF", 4);
    header.length = GUINT32_TO_LE(0);
    memcpy(&header.chunk_type, "WAVE", 4);
    memcpy(&header.sub_chunk, "fmt ", 4);
    header.sc_len = GUINT32_TO_LE(16);
    header.format = GUINT16_TO_LE(1);
    header.modus = GUINT16_TO_LE(input.channels);
    header.sample_fq = GUINT32_TO_LE(input.frequency);
    if (input.format == FMT_U8 || input.format == FMT_S8)
        header.bit_p_spl = GUINT16_TO_LE(8);
    else
        header.bit_p_spl = GUINT16_TO_LE(16);
    header.byte_p_sec = GUINT32_TO_LE(input.frequency * header.modus * (GUINT16_FROM_LE(header.bit_p_spl) / 8));
    header.byte_p_spl = GUINT16_TO_LE((GUINT16_FROM_LE(header.bit_p_spl) / (8 / input.channels)));
    memcpy(&header.data_chunk, "data", 4);
    header.data_length = GUINT32_TO_LE(0);
    aud_vfs_fwrite(&header, sizeof (struct wavhead), 1, output_file);

    return 1;
}

static void wav_write(void *ptr, gint length)
{
    written += aud_vfs_fwrite(ptr, 1, length, output_file);
}

static void wav_flush(void)
{
    //nothing to do here yet. --AOS
}

static void wav_close(void)
{
    if (output_file)
    {
        header.length = GUINT32_TO_LE(written + sizeof (struct wavhead) - 8);

        header.data_length = GUINT32_TO_LE(written);
        aud_vfs_fseek(output_file, 0, SEEK_SET);
        aud_vfs_fwrite(&header, sizeof (struct wavhead), 1, output_file);
    }
}

static gint wav_free(void)
{
    return 1000000;
}

static gint wav_playing(void)
{
    return 0;
}

static gint wav_get_written_time(void)
{
    if (header.byte_p_sec != 0)
        return (gint) ((written * 1000) / header.byte_p_sec + offset);
    return 0;
}
