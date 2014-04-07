/*
 *  T50 - Experimental Mixed Packet Injector
 *
 *  Copyright (C) 2010 - 2014 - T50 developers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

/* Function Name: EGP packet header configuration.

Description:   This function configures and sends the EGP packet header.

Targets:       N/A */
void egp(worker_data_t *data)
{
  size_t greoptlen;   /* GRE options size. */

  struct iphdr * ip;

  /* EGP header and EGP acquire header. */
  struct egp_hdr * egp;
  struct egp_acq_hdr * egp_acq;

  struct config_options *co;

  assert(data != NULL);

  co = data->co;

  greoptlen = gre_opt_len(co->gre.options, co->encapsulated);
  data->upktsize = sizeof(struct iphdr)   +
          greoptlen              +
          sizeof(struct egp_hdr) +
          sizeof(struct egp_acq_hdr);

  /* Try to reallocate packet, if necessary */
  alloc_packet(data);

  /* IP Header structure making a pointer to Packet. */
  ip = ip_header(data);

  /* GRE Encapsulation takes place. */
  gre_encapsulation(data,
        sizeof(struct iphdr)    +
        sizeof(struct egp_hdr)  +
        sizeof(struct egp_acq_hdr));

  /*
   * @nbrito -- Tue Jan 18 11:09:34 BRST 2011
   * XXX Have to work a little bit more deeply in packet building.
   * XXX Checking EGP Type and building appropriate header.
   */
  /* EGP Header structure making a pointer to Packet. */
  egp           = (struct egp_hdr *)((void *)ip + sizeof(struct iphdr) + greoptlen);
  egp->version  = EGPVERSION;
  egp->type     = co->egp.type;
  egp->code     = co->egp.code;
  egp->status   = co->egp.status;
  egp->as       = __RND(co->egp.as);
  egp->sequence = __RND(co->egp.sequence);
  egp->check    = 0;

  /* EGP Acquire Header structure. */
  egp_acq        = (struct egp_acq_hdr *)((void *)egp + sizeof(struct egp_hdr));
  egp_acq->hello = __RND(co->egp.hello);
  egp_acq->poll  = __RND(co->egp.poll);

  /* Computing the checksum. */
  egp->check    = co->bogus_csum ? __RND(0) : 
    cksum(egp, sizeof(struct egp_hdr) + sizeof(struct egp_acq_hdr));

  /* GRE Encapsulation takes place. */
  gre_checksum(data);
}
