/*
 * mx is a small programs that prints out the mx records
 * for a particulary domain
 */

#include <stdio.h>
#include <config.h>
#include <ldns/ldns.h>

int
usage(FILE *fp, char *prog) {
	fprintf(fp, "%s keygen\n", prog);
	fprintf(fp, "  generate a DNSKEY RR \n");
	return 0;
}

int
main()
{
	ldns_rr *dnskey;
	ldns_key *privkey;
	ldns_rr *dnskey_dsa;
	ldns_key *privkey_dsa;
	ldns_rdf *owner;
	ldns_rr *rr;
	ldns_key_list *keys;
	ldns_rr_list  *rrs;
	ldns_rr_list  *signatures;
	ldns_rr_list  *dnskeys;

	keys = ldns_key_list_new();
	rrs  = ldns_rr_list_new();
	dnskeys = ldns_rr_list_new();

	/* well formed */
	rr = ldns_rr_new_frm_str("a.miek.nl. 1800 IN A 195.169.222.38");
	ldns_rr_print(stdout, rr);
	printf("\n");

	rr = ldns_rr_new_frm_str("a.miek.nl. 1800 IN MX 10 www.atoom.net");
	ldns_rr_print(stdout, rr);
	printf("\n");
	
	/* miss formed */
	rr = ldns_rr_new_frm_str("a.miek.nl. 1800 IN MX 10");
	ldns_rr_print(stdout, rr);
	printf("\n");

	rr = ldns_rr_new_frm_str("a.miek.nl. 1800 IN A 267.271.122.1t");
	ldns_rr_print(stdout, rr);
	printf("\n");
	exit(0);

	privkey = ldns_key_new_frm_algorithm(LDNS_SIGN_RSASHA1, 512);
	privkey_dsa = ldns_key_new_frm_algorithm(LDNS_SIGN_DSA, 512);
	if (!privkey || !privkey_dsa) {
		printf("Ah, keygen failed");
		exit(1);
	}

	owner = ldns_dname_new_frm_str("miek.nl");
	ldns_key_set_pubkey_owner(privkey, owner);
	ldns_key_set_pubkey_owner(privkey_dsa, owner);

	ldns_key_set_origttl(privkey, 1800);
	ldns_key_set_origttl(privkey_dsa, 1800);
	/*	SSL_load_error_strings();*/

	ldns_key_list_push_key(keys, privkey);
	/* ldns_key_list_push_key(keys, privkey_dsa); */

	ldns_rr_list_push_rr(rrs, rr);
	
	dnskey = ldns_key2rr(privkey);
	dnskey_dsa = ldns_key2rr(privkey_dsa);
	if (dnskey) {
		ldns_rr_print(stdout, dnskey);
		printf("; {%d}\n", 
				(int) ldns_calc_keytag(dnskey));
		printf("\n");
		ldns_key_set_keytag(privkey, ldns_calc_keytag(dnskey));
	} else {
		exit(1);
	}
	if (dnskey_dsa) {
		ldns_rr_print(stdout, dnskey_dsa);
		printf("; {%d}\n", 
				(int) ldns_calc_keytag(dnskey_dsa));
		printf("\n");
		ldns_key_set_keytag(privkey_dsa, ldns_calc_keytag(dnskey_dsa));
	} else {
		exit(1);
	}
	/* no verify */
	ldns_rr_list_push_rr(dnskeys, dnskey);
	ldns_rr_list_push_rr(dnskeys, dnskey_dsa); 

	signatures = ldns_sign_public(rrs, keys);

	ldns_rr_list_print(stdout, signatures);

	printf("Now we are going to verify\n");

	printf("\n[%d]\n", ldns_verify(rrs, signatures, dnskeys));
	
        return 0;
}
