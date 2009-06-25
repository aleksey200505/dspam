delete from dspam_token_data 
  where (innocent_hits*2) + spam_hits < 5
  and date('now')-date(last_hit) > 30;
delete from dspam_token_data
  where innocent_hits + spam_hits = 1
  and date('now')-date(last_hit) > 15;
delete from dspam_token_data
  where date('now')-date(last_hit) > 90;
delete from dspam_signature_data
  where date('now')-date(created_on) > 14;
