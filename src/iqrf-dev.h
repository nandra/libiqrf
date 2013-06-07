/*
 * Public libiqrf header file
 * Copyright (C) 2010 Marek Belisko <marek.belisko@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef IQRF_DEV_H
#define IQRF_DEV_H

#include <stdint.h>
#include <semaphore.h>
#include "usb.h"


typedef struct {
	device_t *dev;
	sem_t sem;

} iqrf_t;

/* TODO */
#if 0
/* IQRF packet structure */
typedef struct {
	unsigned char rx_buff[64];
	unsigned char tx_buff[64];


} iqpac_t
#endif

#endif // IQRF_DEV_H
