-- ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
-- +                                                                +
-- +                       Kitty Comfort                            +
-- +                    Data Storage Module                         +
-- +                                                                +
-- ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

-- Copyright 2021 Tyler J. Anderson

-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:

-- 1. Redistributions of source code must retain the above copyright
-- notice, this list of conditions and the following disclaimer.

-- 2. Redistributions in binary form must reproduce the above
-- copyright notice, this list of conditions and the following
-- disclaimer in the documentation and/or other materials provided
-- with the distribution.

-- 3. Neither the name of the copyright holder nor the names of its
-- contributors may be used to endorse or promote products derived
-- from this software without specific prior written permission.

-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
-- FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
-- COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
-- INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
-- SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
-- HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
-- STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
-- OF THE POSSIBILITY OF SUCH DAMAGE.

-- kittyview.sql

-- SQL Statements to prepare the database for data upload and generate
-- a view that will attach useable timestamps to the logged data

-- Create the main logging table and add required columns
create table if not exists kittyfiler.ammonia
  (
    LID serial,
    sentmillis bigint,
    timemillis bigint,
    value numeric,
    warmedup bool,
    readtime timestamptz
  );

-- Build view that estimates time stamp from the microcontroller's
-- Millis readings
CREATE OR REPLACE VIEW kittyfiler.kittyview as
select sq1.LID, sq1.timemillis, sq1.value, sq1.warmedup,
       to_timestamp(sq1.timemillis *
		    regr_slope(
		      sq1.readtime_epoch,
		      sq1.sentmillis) over w1 +
		      regr_intercept(
			sq1.readtime_epoch,
			sq1.sentmillis) over w1)
	 as esttime
  from (
    select LID, sentmillis, timemillis, value, warmedup, readtime,
	   sum(startpart)
	     over (order by LID rows between unbounded preceding and current row)
	     as timegroups,
	   extract(epoch from readtime) as readtime_epoch
      from (
	select LID, sentmillis, timemillis, value, warmedup, readtime,
	       case when lag(sentmillis::int, 1, 0) over (order by LID ASC)
			> sentmillis then 1 else 0
	       end::integer as startpart
	  from kittyfiler.ammonia) d1
     order by LID) sq1
	 window w1 as (partition by sq1.timegroups)
 order by sq1.LID;
