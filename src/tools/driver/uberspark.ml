(* uberspark config tool: to locate hwm, libraries and headers *)
(* author: amit vasudevan (amitvasudevan@acm.org) *)

open Sys
open Unix
open Filename

open Uslog
open Usosservices
open Libusmf
open Usuobjlib
open Usuobj
open Usuobjcollection

let log_mpf = "uberSpark";;

let g_install_prefix = "/usr/local";;
let g_uberspark_install_bindir = "/usr/local/bin";;
let g_uberspark_install_homedir = "/usr/local/uberspark";;
let g_uberspark_install_includedir = "/usr/local/uberspark/include";;
let g_uberspark_install_hwmdir = "/usr/local/uberspark/hwm";;
let g_uberspark_install_hwmincludedir = "/usr/local/uberspark/hwm/include";;
let g_uberspark_install_libsdir = "/usr/local/uberspark/libs";;
let g_uberspark_install_libsincludesdir = "/usr/local/uberspark/libs/include";;
let g_uberspark_install_toolsdir = "/usr/local/uberspark/tools";;


(*----------------------------------------------------------------------------*)
(* command line options *)
let cmdopt_invalid opt = 
	Uslog.logf log_mpf Uslog.Info "invalid option: '%s'; use -help to see available options" opt;
	ignore(exit 1);
	;;

let copt_builduobj = ref false;;

let cmdopt_uobjlist = ref "";;
let cmdopt_uobjlist_set value = cmdopt_uobjlist := value;;

let cmdopt_uobjmanifest = ref "";;
let cmdopt_uobjmanifest_set value = cmdopt_uobjmanifest := value;;
(*----------------------------------------------------------------------------*)


let uberspark_build_includedirs_base () = 
  let p_output = ref [] in
		p_output := !p_output @ [ "-I" ];
		p_output := !p_output @ [ g_uberspark_install_includedir ];
		p_output := !p_output @ [ "-I" ];
		p_output := !p_output @ [ g_uberspark_install_hwmincludedir ];
		p_output := !p_output @ [ "-I" ];
		p_output := !p_output @ [ g_uberspark_install_libsincludesdir ];
		(!p_output)		
;;	

let uberspark_build_includedirs uobj_id uobj_hashtbl_includedirs = 
  let p_output = ref [] in
	let uobj_hashtbl_includedirs_list = (Hashtbl.find_all uobj_hashtbl_includedirs uobj_id) in 
		List.iter (fun x -> p_output := !p_output @ [ "-I" ] @ [ x ]) uobj_hashtbl_includedirs_list;
	(!p_output)		
;;	




(*								
let uberspark_link_uobj uobj_cfile_list uobj_libdirs_list uobj_libs_list 
		uobj_linker_script uobj_bin_name = 
		let ld_cmdline = ref [] in
			ld_cmdline := !ld_cmdline @ [ "--oformat" ];
			ld_cmdline := !ld_cmdline @ [ "binary" ];
			ld_cmdline := !ld_cmdline @ [ "-T" ];
			ld_cmdline := !ld_cmdline @ [ uobj_linker_script ]; 
			List.iter (fun x -> ld_cmdline := !ld_cmdline @ [ (x^".o") ]) uobj_cfile_list; 
			ld_cmdline := !ld_cmdline @ [ "-o" ];
			ld_cmdline := !ld_cmdline @ [ uobj_bin_name ];
			List.iter (fun x -> ld_cmdline := !ld_cmdline @ [ ("-L"^x) ]) uobj_libdirs_list; 
			ld_cmdline := !ld_cmdline @ [ "--start-group" ];
			List.iter (fun x -> ld_cmdline := !ld_cmdline @ [ ("-l"^x) ]) uobj_libs_list; 
			ld_cmdline := !ld_cmdline @ [ "--end-group" ];
			let (pestatus, pesignal, poutput) = 
				(exec_process_withlog g_uberspark_exttool_ld !ld_cmdline true) in
						if (pesignal == true) || (pestatus != 0) then
							begin
									Uslog.logf log_mpf Uslog.Error "in linking uobj binary '%s'!" uobj_bin_name;
									ignore(exit 1);
							end
						else
							begin
									Uslog.logf log_mpf Uslog.Info "Linked uobj binary '%s' successfully" uobj_bin_name;
							end
						;
		()
;;

																
								
																
*)
																								
														
(*----------------------------------------------------------------------------*)
																																
																																																																				
																																																
let main () =
		let speclist = [
			("--builduobj", Arg.Set copt_builduobj, "Build uobj binary by compiling and linking");
			("-b", Arg.Set copt_builduobj, "Build uobj binary by compiling and linking");
			("--uobjlist", Arg.String (cmdopt_uobjlist_set), "uobj list filename with path");
			("--uobjmanifest", Arg.String (cmdopt_uobjmanifest_set), "uobj list filename with path");

			] in
		let banner = "uberSpark driver tool by Amit Vasudevan (amitvasudevan@acm.org)" in
		let usage_msg = "Usage:" in
		let uobj_id = ref 0 in
		let uobj_manifest_filename = ref "" in
		let uobj_name = ref "" in
		let uobj_mf_filename_forpreprocessing = ref "" in	
		let uobj_mf_filename_preprocessed = ref "" in  
			
		(* set debug verbosity *)
		Uslog.current_level := Uslog.ord Uslog.Debug; 

	  (* print banner and parse command line args *)
		Uslog.logf log_mpf Uslog.Info "%s" banner;
		Uslog.logf log_mpf Uslog.Info ">>>>>>";
		Arg.parse speclist cmdopt_invalid usage_msg;

		(* build uobj collection *)
		Uslog.logf log_mpf Uslog.Info "Proceeding to build uobj collection using: %s..." !cmdopt_uobjlist;
		Usuobjcollection.build !cmdopt_uobjlist "" true;
		(*Libusmf.usmf_parse_uobj_list (!cmdopt_uobjlist) ((Filename.dirname !cmdopt_uobjlist) ^ "/");*)
		Uslog.logf log_mpf Uslog.Info "Built uobj collection, total uobjs=%u" !Usuobjcollection.total_uobjs;

		(* grab uobj manifest filename and derive uobj name *)
		uobj_manifest_filename := (Filename.basename !cmdopt_uobjmanifest);
		uobj_name := Filename.chop_extension !uobj_manifest_filename;

		(* check options and do the task *)
		if (!copt_builduobj == true ) then
			begin
				let uobj = new Usuobj.uobject in
					uobj#build !uobj_manifest_filename "" true	
			end
		;

(*			uobj_id := (Hashtbl.find Libusmf.slab_nametoid !uobj_name);*)

(*
			Uslog.logf log_mpf Uslog.Info "Parsing uobj manifest using: %s..." !cmdopt_uobjmanifest;
			Uslog.logf log_mpf Uslog.Info "uobj_name='%s', uobj_id=%u" !uobj_name !uobj_id;

			if (Libusmf.usmf_parse_uobj_mf_uobj_sources !uobj_id !cmdopt_uobjmanifest) == false then
				begin
					Uslog.logf log_mpf Uslog.Error "invalid or no uobj-sources node found within uobj manifest.";
					ignore (exit 1);
				end
			;

			Uslog.logf log_mpf Uslog.Info "Parsed uobj-sources from uobj manifest.";
			Uslog.logf log_mpf Uslog.Info "incdirs=%u, incs=%u, libdirs=%u, libs=%u"
				(List.length (Hashtbl.find_all Libusmf.slab_idtoincludedirs !uobj_id))
				(List.length (Hashtbl.find_all Libusmf.slab_idtoincludes !uobj_id))
				(List.length (Hashtbl.find_all Libusmf.slab_idtolibdirs !uobj_id))
				(List.length (Hashtbl.find_all Libusmf.slab_idtolibs !uobj_id))
				;
			Uslog.logf log_mpf Uslog.Info "cfiles=%u, casmfiles=%u, asmfiles=%u"
				(List.length (Hashtbl.find_all Libusmf.slab_idtocfiles !uobj_id))
				(List.length (Hashtbl.find_all Libusmf.slab_idtocasmfiles !uobj_id))
				(List.length (Hashtbl.find_all Libusmf.slab_idtoasmfiles !uobj_id))
				;
	
			uobj_mf_filename_forpreprocessing := 
					uberspark_generate_uobj_mf_forpreprocessing !uobj_id 
						!uobj_manifest_filename Libusmf.slab_idtoincludes;
			Uslog.logf log_mpf Uslog.Info "Generated uobj manifest file for preprocessing";
						
			uobj_mf_filename_preprocessed := 
					uberspark_generate_uobj_mf_preprocessed !uobj_id
					!uobj_mf_filename_forpreprocessing 
					(uberspark_build_includedirs_base () @ 
					(uberspark_build_includedirs !uobj_id Libusmf.slab_idtoincludedirs));	
			Uslog.logf log_mpf Uslog.Info "Pre-processed uobj manifest file";

	
			let (rval, uobj_sections_list) = 
				Libusmf.usmf_parse_uobj_mf_uobj_binary !uobj_id !uobj_mf_filename_preprocessed in
					if (rval == false) then
						begin
							Uslog.logf log_mpf Uslog.Error "invalid or no uobj-binary node found within uobj manifest.";
							ignore (exit 1);
						end
					;

			Uslog.logf log_mpf Uslog.Info "Parsed uobj-binary from uobj manifest: total sections=%u"
				(List.length uobj_sections_list);
		

				
			let uobj_hdr_cfile = uberspark_generate_uobj_hdr !uobj_name 0x80000000 
				uobj_sections_list in
				Uslog.logf log_mpf Uslog.Info "Generated uobj header file";
			
							
												
																				
			if (List.length (Hashtbl.find_all Libusmf.slab_idtocfiles !uobj_id)) > 0 then
				begin
					Uslog.logf log_mpf Uslog.Info "Proceeding to compile uobj cfiles...";
					uberspark_compile_uobj_cfiles 
						((Hashtbl.find_all Libusmf.slab_idtocfiles !uobj_id) @ [ uobj_hdr_cfile ])
						(uberspark_build_includedirs_base () @ 
						(uberspark_build_includedirs !uobj_id Libusmf.slab_idtoincludedirs));
				end
			;


			uberspark_generate_uobj_linker_script !uobj_name 0x80000000 
				uobj_sections_list;
		


			Uslog.logf log_mpf Uslog.Info "Proceeding to link uobj binary '%s'..."
				!uobj_name;
			let uobj_libdirs_list = ref [] in
			let uobj_libs_list = ref [] in
				uobj_libdirs_list := !uobj_libdirs_list @ [ g_uberspark_install_libsdir ]; 	
				uobj_libdirs_list := !uobj_libdirs_list @
								(Hashtbl.find_all Libusmf.slab_idtolibdirs !uobj_id);
				uobj_libs_list := !uobj_libs_list @
								(Hashtbl.find_all Libusmf.slab_idtolibs !uobj_id);
				uberspark_link_uobj ((Hashtbl.find_all Libusmf.slab_idtocfiles !uobj_id) @ [ uobj_hdr_cfile ])
					!uobj_libdirs_list !uobj_libs_list 
					(!uobj_name ^ ".lscript") !uobj_name;

*)
			
(*							
			Usuobjlib.build !uobj_manifest_filename "" true;*)
 ;;

		
main ();;



(*
			file_copy !cmdopt_uobjmanifest (!uobj_name ^ ".gsm.pp");

			Uslog.logf log_mpf Uslog.Info "uobj_name=%s, uobj_id=%u\n"  !uobj_name !uobj_id;
			Libusmf.usmf_memoffsets := false;
			Libusmf.usmf_parse_uobj_mf (Hashtbl.find Libusmf.slab_idtogsm !uobj_id) (Hashtbl.find Libusmf.slab_idtommapfile !uobj_id);
*)


(*
			Uslog.current_level := Uslog.ord Uslog.Info;
			Uslog.logf log_mpf Uslog.Info "proceeding to execute...\n";

			let p_cmdline = ref [] in
				p_cmdline := !p_cmdline @ [ "gcc" ];
				p_cmdline := !p_cmdline @ [ "-P" ];
				p_cmdline := !p_cmdline @ [ "-E" ];
				p_cmdline := !p_cmdline @ [ "../../dat.c" ];
				p_cmdline := !p_cmdline @ [ "-o" ];
				p_cmdline := !p_cmdline @ [ "dat.i" ];
				
			let (exit_status, exit_signal, process_output) = (exec_process_withlog "gcc" !p_cmdline true) in
						Uslog.logf log_mpf Uslog.Info "Done: exit_signal=%b exit_status=%d\n" exit_signal exit_status;
*)

(*
				let str_list = (Hashtbl.find_all Libusmf.slab_idtoincludedirs !uobj_id) in
				begin
					Uslog.logf log_mpf Uslog.Info "length=%u\n"  (List.length str_list);
					while (!i < (List.length str_list)) do
						begin
							let mstr = (List.nth str_list !i) in
							Uslog.logf log_mpf Uslog.Info "i=%u --> %s" !i mstr; 
							i := !i + 1;
						end
					done;
				end
*)

(*
		Uslog.logf log_mpf Uslog.Info "proceeding to parse includes...";
			Libusmf.usmf_parse_uobj_mf_includedirs !uobj_id !cmdopt_uobjmanifest;
			Uslog.logf log_mpf Uslog.Info "includes parsed.";

			let str_list = (Hashtbl.find_all Libusmf.slab_idtoincludedirs !uobj_id) in
				begin
					Uslog.logf log_mpf Uslog.Info "length=%u\n"  (List.length str_list);
					while (!i < (List.length str_list)) do
						begin
							let include_dir_str = (List.nth str_list !i) in
							Uslog.logf log_mpf Uslog.Info "i=%u --> %s" !i include_dir_str; 
							i := !i + 1;
						end
					done;
				end
*)
