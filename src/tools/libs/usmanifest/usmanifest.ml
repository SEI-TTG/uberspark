(*----------------------------------------------------------------------------*)
(* uberSpark manifest interface *)
(*	 author: amit vasudevan (amitvasudevan@acm.org) *)
(*----------------------------------------------------------------------------*)

open Yojson


open Usconfig
open Uslog
open Usosservices
open Usextbinutils


module Usmanifest =
	struct

	let log_tag = "Usmanifest";;

	(*--------------------------------------------------------------------------*)
	(* read manifest file into json object *)
	(*--------------------------------------------------------------------------*)

	let read_manifest usmf_filename keep_temp_files = 
		let retval = ref false in
	  let retjson = ref `Null in
		let usmf_filename_in_pp = (usmf_filename ^ ".c") in
		let usmf_filename_out_pp = (usmf_filename ^ ".upp") in
			Usosservices.file_copy usmf_filename usmf_filename_in_pp;
			let (pp_retval, _) = Usextbinutils.preprocess usmf_filename_in_pp 
														usmf_filename_out_pp 
														(Usconfig.get_std_incdirs ())
														(Usconfig.get_std_defines () @ 
															Usconfig.get_std_define_asm ()) in
	
			if(pp_retval == 0) then 
				begin
					try
				
						 let uobj_mf_json = Yojson.Basic.from_file usmf_filename_out_pp in
								retval := true;
								retjson := uobj_mf_json;
								
					with Yojson.Json_error s -> 
							Uslog.logf "libusmf" Uslog.Debug "usmf_read_manifest: ERROR:%s" s;
							retval := false;
					;
					
					if(keep_temp_files == false) then
						begin
							Usosservices.file_remove usmf_filename_in_pp;
							Usosservices.file_remove usmf_filename_out_pp;
						end
					;
				end
			;	
	
		(!retval, !retjson)
	;;


	(*--------------------------------------------------------------------------*)
	(* parse manifest node "usmf-hdr" *)
	(* return: true if successfully parsed usmf-hdr, false if not *)
	(* if true also return: manifest type string; manifest subtype string; *)
	(* id as string *)
	(*--------------------------------------------------------------------------*)

	let parse_node_usmf_hdr usmf_json =
		let retval = ref false in
		let usmf_hdr_type = ref "" in
		let usmf_hdr_subtype = ref "" in
		let usmf_hdr_id = ref "" in
		let usmf_hdr_platform = ref "" in
		let usmf_hdr_cpu = ref "" in
		let usmf_hdr_arch = ref "" in
		try
			let open Yojson.Basic.Util in
				let usmf_json_usmf_hdr = usmf_json |> member "usmf-hdr" in
				if(usmf_json_usmf_hdr <> `Null) then
					begin
						usmf_hdr_type := usmf_json_usmf_hdr |> member "type" |> to_string;
						usmf_hdr_subtype := usmf_json_usmf_hdr |> member "subtype" |> to_string;
						usmf_hdr_id := usmf_json_usmf_hdr |> member "id" |> to_string;
						usmf_hdr_platform := usmf_json_usmf_hdr |> member "platform" |> to_string;
						usmf_hdr_cpu := usmf_json_usmf_hdr |> member "cpu" |> to_string;
						usmf_hdr_arch := usmf_json_usmf_hdr |> member "arch" |> to_string;
						retval := true;
					end
				;

		with Yojson.Basic.Util.Type_error _ -> 
				retval := false;
		;

		(!retval, !usmf_hdr_type, !usmf_hdr_subtype, !usmf_hdr_id,
			!usmf_hdr_platform, !usmf_hdr_cpu, !usmf_hdr_arch)
	;;



	(*--------------------------------------------------------------------------*)
	(* parse manifest node "uobjlib-sources" *)
	(* return true on successful parse, false if not *)
	(* return: if true then lists of c-files and casm files *)
	(*--------------------------------------------------------------------------*)
	let parse_node_usmf_sources usmf_json =
		let retval = ref true in
		let usmf_cfiles_list = ref [] in
		let usmf_casmfiles_list = ref [] in

		try
			let open Yojson.Basic.Util in
		  	let usmf_sources_json = usmf_json |> member "usmf-sources" in
					if usmf_sources_json != `Null then
						begin
	
							let usmf_cfiles_json = usmf_sources_json |> 
								member "c-files" in
								if usmf_cfiles_json != `Null then
									begin
										let usmf_cfiles_json_list = usmf_cfiles_json |> 
												to_list in 
											List.iter (fun x -> usmf_cfiles_list := 
													!usmf_cfiles_list @ [(x |> to_string)]
												) usmf_cfiles_json_list;
									end
								;

							let usmf_casmfiles_json = usmf_sources_json |> 
								member "casm-files" in
								if usmf_casmfiles_json != `Null then
									begin
										let usmf_casmfiles_json_list = usmf_casmfiles_json |> 
												to_list in 
											List.iter (fun x -> usmf_casmfiles_list := 
													!usmf_casmfiles_list @ [(x |> to_string)]
												) usmf_casmfiles_json_list;
									end
								;

						end
					;
					
		with Yojson.Basic.Util.Type_error _ -> 
				retval := false;
		;
	
		(!retval, !usmf_cfiles_list, !usmf_casmfiles_list)
	;;


	(*--------------------------------------------------------------------------*)
	(* parse manifest node "usmf-vharness" *)
	(* return true if vharness node present, false if not *)
	(* return: if true then returns list of lists of vharnesses *)
	(*--------------------------------------------------------------------------*)
	
	let parse_node_usmf_vharness usmf_json =
		let retval = ref false in
		let usmf_vharness_list = ref [] in

		try
			let open Yojson.Basic.Util in
		  	let usmf_vharness_json = usmf_json |> member "usmf-vharness" in
					if usmf_vharness_json != `Null then
						begin

							let usmf_vharness_assoc_list = Yojson.Basic.Util.to_assoc 
									usmf_vharness_json in
								
								retval := true;
								List.iter (fun (x,y) ->
									Uslog.logf "usmanifest" Uslog.Debug "%s: key=%s" __LOC__ x;
									let usmf_vharness_attribute_list = ref [] in
										usmf_vharness_attribute_list := !usmf_vharness_attribute_list @
																	[ x ];
										List.iter (fun z ->
											usmf_vharness_attribute_list := !usmf_vharness_attribute_list @
																	[ (z |> to_string) ];
											()
										)(Yojson.Basic.Util.to_list y);
										
										usmf_vharness_list := !usmf_vharness_list @	[ !usmf_vharness_attribute_list ];
										(* if (List.length (Yojson.Basic.Util.to_list y)) < 3 then
											retval:=false;
										*)
									()
								) usmf_vharness_assoc_list;
							Uslog.logf "usmanifests" Uslog.Debug "%s: list length=%u" __LOC__ (List.length !usmf_vharness_list);

						end
					;
					
		with Yojson.Basic.Util.Type_error _ -> 
				retval := false;
		;
	
		(!retval, !usmf_vharness_list)
	;;


	(*--------------------------------------------------------------------------*)
	(* parse manifest node "uobj-binary" *)
	(* return true on successful parse, false if not *)
	(* return: if true then list of sections *)
	(*--------------------------------------------------------------------------*)
	let parse_node_uobj_binary usmf_json =
		let retval = ref false in
		let uobj_sections_list = ref [] in

		try
			let open Yojson.Basic.Util in
		  	let uobj_binary_json = usmf_json |> member "uobj-binary" in
					if uobj_binary_json != `Null then
						begin

							let uobj_sections_json = uobj_binary_json |> member "uobj-sections" in
								if uobj_sections_json != `Null then
									begin
										let uobj_sections_assoc_list = Yojson.Basic.Util.to_assoc uobj_sections_json in
											retval := true;
											List.iter (fun (x,y) ->
													Uslog.logf log_tag Uslog.Debug "%s: key=%s" __LOC__ x;
													let uobj_section_attribute_list = ref [] in
														uobj_section_attribute_list := !uobj_section_attribute_list @
																					[ x ];
														List.iter (fun z ->
															uobj_section_attribute_list := !uobj_section_attribute_list @
																					[ (z |> to_string) ];
															()
														)(Yojson.Basic.Util.to_list y);
														
														uobj_sections_list := !uobj_sections_list @	[ !uobj_section_attribute_list ];
														if (List.length (Yojson.Basic.Util.to_list y)) < 3 then
															retval:=false;
													()
												) uobj_sections_assoc_list;
											Uslog.logf log_tag Uslog.Debug "%s: list length=%u" __LOC__ (List.length !uobj_sections_list);
									end
								;		
					
						end
					;
															
		with Yojson.Basic.Util.Type_error _ -> 
				retval := false;
		;

								
		(!retval, !uobj_sections_list)
	;;

																								
																																																
	(*--------------------------------------------------------------------------*)
	(* parse manifest node "uobj-sentinels" *)
	(* return true on successful parse, false if not *)
	(* return: if true then list of sections *)
	(*--------------------------------------------------------------------------*)
	let parse_node_uobj_sentinels usmf_json =
		let retval = ref false in
		let uobj_sentinels_list = ref [] in

		try
			let open Yojson.Basic.Util in
		  	let uobj_sentinels_json = usmf_json |> member "uobj-sentinels" in
					if uobj_sentinels_json != `Null then
						begin

							let uobj_sentinels_assoc_list = Yojson.Basic.Util.to_assoc uobj_sentinels_json in
								retval := true;
								List.iter (fun (x,y) ->
										Uslog.logf log_tag Uslog.Debug "%s: key=%s" __LOC__ x;
										let uobj_sentinels_attribute_list = ref [] in
											uobj_sentinels_attribute_list := !uobj_sentinels_attribute_list @
																		[ x ];
											List.iter (fun z ->
												uobj_sentinels_attribute_list := !uobj_sentinels_attribute_list @
																		[ (z |> to_string) ];
												()
											)(Yojson.Basic.Util.to_list y);
											
											uobj_sentinels_list := !uobj_sentinels_list @	[ !uobj_sentinels_attribute_list ];
											if (List.length (Yojson.Basic.Util.to_list y)) < 3 then
												retval:=false;
										()
									) uobj_sentinels_assoc_list;
								Uslog.logf log_tag Uslog.Debug "%s: list length=%u" __LOC__ (List.length !uobj_sentinels_list);

						end
					;
															
		with Yojson.Basic.Util.Type_error _ -> 
				retval := false;
		;

								
		(!retval, !uobj_sentinels_list)
	;;
																																																																																																
			
	(*--------------------------------------------------------------------------*)
	(* parse manifest node "uobj-coll" *)
	(* return true on successful parse, false if not *)
	(* return: if true then list of uobj directories *)
	(*--------------------------------------------------------------------------*)
	let parse_node_usmf_uobj_coll usmf_json =
		let retval = ref true in
		let usmf_uobj_dirs_list = ref [] in

		try
			let open Yojson.Basic.Util in
		  	let uobj_coll_json = usmf_json |> member "uobj-coll" in
					if uobj_coll_json != `Null then
						begin
							let usmf_uobj_coll_json_list = uobj_coll_json |> 
									to_list in 
								List.iter (fun x -> usmf_uobj_dirs_list := 
										!usmf_uobj_dirs_list @ [(x |> to_string)]
									) usmf_uobj_coll_json_list;
						end
					;
	
		with Yojson.Basic.Util.Type_error _ -> 
				retval := false;
		;
	
		(!retval, !usmf_uobj_dirs_list)
	;;
																								
																																													
																																																																																							
	end