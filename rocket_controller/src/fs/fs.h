/** 
 * @file fs.h
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-17
 * @brief Header file for the various file system implementations
 */


#ifndef FS_H
#define FS_H

#include "microshell.h"

#include <stdint.h>
#include <stdbool.h>

/** 
 * @brief Mount the bin (executables directory)
 * @param ush the ush object to mount to
 * 
 */
void fs_mnt_bin(struct ush_object *ush);

void fs_mnt_dev(struct ush_object *ush);

void fs_mnt_etc(struct ush_object *ush);

void fs_mnt_mnt(struct ush_object *ush);

void fs_mnt_log(struct ush_object *ush);

#endif // FS_H