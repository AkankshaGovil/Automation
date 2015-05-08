BEGIN { rpc_specific = 0; }
/(xdr_)|(rpc\.h)/ { if(rpc_specific == 0) print "#ifdef RPC_SPECIFIC"; print $0;
	rpc_specific = 1; }
!/(xdr_)|(rpc\.h)/ { if(rpc_specific == 1) print"#endif"; print $0; rpc_specific = 0; }
