`timescale 1ns / 1ps

module tb_rx_parser;

    // Parameters
    parameter DATA_W = 64;
    
    // Inputs
    reg clk;
    reg rst;
    reg [DATA_W-1:0] s_axis_tdata;
    reg s_axis_tvalid;
    reg s_axis_tlast;
    reg m_axis_tready;

    // Outputs
    wire s_axis_tready;
    wire [DATA_W-1:0] m_axis_tdata;
    wire m_axis_tvalid;
    wire m_axis_tlast;
    wire [31:0] ip_src;
    wire [31:0] ip_dst;
    wire [15:0] udp_sport;
    wire [15:0] udp_dport;
    wire header_valid;

    // Instantiate the Unit Under Test (UUT)
    rx_parser #(
        .DATA_W(DATA_W)
    ) uut (
        .clk(clk), 
        .rst(rst), 
        .s_axis_tdata(s_axis_tdata), 
        .s_axis_tvalid(s_axis_tvalid), 
        .s_axis_tlast(s_axis_tlast), 
        .s_axis_tready(s_axis_tready), 
        .m_axis_tdata(m_axis_tdata), 
        .m_axis_tvalid(m_axis_tvalid), 
        .m_axis_tlast(m_axis_tlast), 
        .m_axis_tready(m_axis_tready), 
        .ip_src(ip_src), 
        .ip_dst(ip_dst), 
        .udp_sport(udp_sport), 
        .udp_dport(udp_dport), 
        .header_valid(header_valid)
    );

    // Clock generation
    initial begin
        clk = 0;
        forever #5 clk = ~clk; // 100MHz
    end

    // Test Sequence
    initial begin
        // Initialize Inputs
        rst = 1;
        s_axis_tdata = 0;
        s_axis_tvalid = 0;
        s_axis_tlast = 0;
        m_axis_tready = 1; // Always ready to receive payload

        // Wait 100 ns for global reset to finish
        #100;
        rst = 0;
        #20;

        // --- Send Packet ---
        // Cycle 1: Ethernet Header (DstMAC + SrcMAC partial) -> Logic assumes skip
        @(posedge clk);
        s_axis_tvalid = 1;
        s_axis_tdata = 64'hAA_BB_CC_DD_EE_FF_11_22; // Garbage MACs
        
        // Cycle 2: IP Header starts (Logic expects IP Src/Dst here)
        // SrcIP: 192.168.1.1 (C0 A8 01 01)
        // DstIP: 224.1.1.1 (E0 01 01 01) (Multicast)
        @(posedge clk);
        s_axis_tdata = 64'hC0A80101_E0010101; 
        
        // Cycle 3: UDP Header (Sport, Dport)
        // Sport: 1234 (04D2)
        // Dport: 8080 (1F90)
        // Len/Chksum garbage
        @(posedge clk);
        s_axis_tdata = 64'h04D2_1F90_0000_0000;
        
        // Cycle 4: Payload Word 1
        @(posedge clk);
        s_axis_tdata = 64'hDEAD_BEEF_CAFE_BABE;
        
        // Cycle 5: Payload Word 2 (Last)
        @(posedge clk);
        s_axis_tdata = 64'h1111_2222_3333_4444;
        s_axis_tlast = 1;
        
        @(posedge clk);
        s_axis_tvalid = 0;
        s_axis_tlast = 0;

        // Wait and Check
        #50;
        
        // Check results (Self-Checking)
        if (ip_src == 32'hC0A80101 && ip_dst == 32'hE0010101) 
            $display("PASS: IP Addresses Parsed Correctly");
        else 
            $display("FAIL: IP Address Mismatch. Src: %h, Dst: %h", ip_src, ip_dst);
            
        if (udp_dport == 16'h1F90)
            $display("PASS: UDP Port Parsed Correctly");
        else
            $display("FAIL: UDP Port Mismatch: %h", udp_dport);

        $finish;
    end
      
endmodule
