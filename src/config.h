/* SIPbot - An opensource VoIP answering machine
 * Copyright (C) 2014-2015 Alain (Carpikes) Carlucci
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file config.h
 * @brief Configuration file
 */

#ifndef CONFIG_H
#define CONFIG_H

/** @brief Registration info */
#define REG_INFO "sip:USERID@sip.messagenet.it"

/** @brief Registrar server */
#define REG_HOST "sip:sip.messagenet.it:5061"

/** @brief Your username */
#define REG_USER "USERID"

/** @brief Your password */
#define REG_PASS "PASSWORD"

/** @brief Client registration timeout */
#define REG_TIMEOUT 3600

/** @brief Call handler */
#define PROGRAM_NAME "./scripts/test.php"
#endif
