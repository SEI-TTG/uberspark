(*------------------------------------------------------------------------------
	uberSpark uberobject collection interface
	author: amit vasudevan (amitvasudevan@acm.org)
------------------------------------------------------------------------------*)

open Usconfig
open Uslog
open Usosservices
open Usmanifest
open Usextbinutils
open Usuobjgen
open Usuobj

module Usuobjcollection =
	struct

	let log_tag = "Usuobjcollection";;
	let total_uobjs = ref 0;;

	let usmf_type_uobjcollection = "uobj_collection";;

	let uobjcoll_rootdir = ref "";;

	(*--------------------------------------------------------------------------*)
	(* build a uobj collection *)
	(* us_manifest_filename = us manifest filename *)
	(* build_dir = directory to use for building *)
	(* keep_temp_files = true if temporary files need to be preserved in build_dir *)
	(*--------------------------------------------------------------------------*)
	let build 
				us_manifest_filename build_dir keep_temp_files = 

		Uslog.logf log_tag Uslog.Info "Starting...";
		
		(* compute the canonical path of the manifest filename *)
		let (retval, retval_path) = Usosservices.abspath us_manifest_filename in
			if (retval == false) then
				begin
					Uslog.logf log_tag Uslog.Error "unable to obtain canonical path for '%s'" us_manifest_filename;
					ignore (exit 1);
				end
			;
		Uslog.logf log_tag Uslog.Info "canonical path=%s" retval_path;		
		
		(* compute root directory of uobj collection manifest *)
		uobjcoll_rootdir := Filename.dirname retval_path;
		Uslog.logf log_tag Uslog.Info "root-dir=%s" !uobjcoll_rootdir;		
		
		
		
		let usmf_type = ref "" in
		let (retval, mf_json) = Usmanifest.read_manifest 
															us_manifest_filename keep_temp_files in
			if (retval == false) then
				begin
					Uslog.logf log_tag Uslog.Error "could not read uobj collection manifest.";
					ignore (exit 1);
				end
			;		

		Uslog.logf log_tag Uslog.Info "Parsed uobj collection manifest.";

		let (rval, usmf_hdr_type, usmf_hdr_subtype, usmf_hdr_id) =
				Usmanifest.parse_node_usmf_hdr mf_json in
			
		if (rval == false) then
			begin
				Uslog.logf log_tag Uslog.Error "invalid manifest hdr.";
				ignore (exit 1);
			end
		;
				
		if (compare usmf_hdr_type usmf_type_uobjcollection) <> 0 then
			begin
				Uslog.logf log_tag Uslog.Error "invalid uobj collection manifest type '%s'." !usmf_type;
				ignore (exit 1);
			end
		;
			
		Uslog.logf log_tag Uslog.Info "Validated uobj collection hdr and manifest type.";
						
						
		let(rval, uobj_dir_list) = 
			Usmanifest.parse_node_usmf_uobj_coll	mf_json in
	
			if (rval == false) then
				begin
					Uslog.logf log_tag Uslog.Error "invalid uobj-coll node in manifest.";
					ignore (exit 1);
				end
			;
				
		Uslog.logf log_tag Uslog.Info "uobj count=%u"
			(List.length uobj_dir_list);

		(* instantiate uobjs *)
		List.iter (fun x ->  
						(* Uslog.logf log_tag Uslog.Info "uobj dir: %s" x; *)
						let (retval, retval_path) = (Usosservices.abspath x) in
							if (retval == false) then
								begin
									Uslog.logf log_tag Uslog.Error "unable to obtain canonical path for '%s'" x;
									ignore (exit 1);
								end
							;
						Uslog.logf log_tag Uslog.Info "entry: %s; canonical path=%s" x retval_path;
		) uobj_dir_list;

		
																																																																																																																																																																																																		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																						
		Uslog.logf log_tag Uslog.Info "Done.";
		()
	;;
								
																								
	end