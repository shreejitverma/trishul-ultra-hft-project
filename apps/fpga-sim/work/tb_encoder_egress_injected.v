`timescale 1ns / 1ps

module tb_encoder_egress;

    reg clk;
    reg rst;
    reg in_valid;
    reg in_buy;
    reg [31:0] in_px;
    reg [31:0] in_qty;
    reg mac_tready;

    wire [63:0] mac_tdata;
    wire mac_tvalid;
    wire mac_tlast;
    wire m_axis_tready;

    // Instantiate Encoder
    wire [63:0] enc_tdata;
    wire enc_tvalid;
    wire enc_tlast;
    wire enc_tready;

    order_encode #(64) encoder (
        .clk(clk), .rst(rst),
        .in_valid(in_valid), .in_buy(in_buy), .in_px(in_px), .in_qty(in_qty),
        .m_axis_tdata(enc_tdata), .m_axis_tvalid(enc_tvalid), 
        .m_axis_tlast(enc_tlast), .m_axis_tready(enc_tready)
    );

    // Instantiate Bridge
    tx_bridge bridge (
        .clk(clk), .rst(rst),
        .enc_tdata(enc_tdata), .enc_tvalid(enc_tvalid), .enc_tlast(enc_tlast), .enc_tready(enc_tready),
        .mac_tdata(mac_tdata), .mac_tvalid(mac_tvalid), .mac_tlast(mac_tlast), .mac_tready(mac_tready)
    );

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        rst = 1;
        in_valid = 0;
        in_buy = 0;
        in_px = 0;
        in_qty = 0;
        mac_tready = 1;
        
        #100;
        rst = 0;
        #20;

        // 1. Generate a BUY order
        @(posedge clk);
        in_valid = 1; in_buy = 1; in_px = 10050; in_qty = 500;
        @(posedge clk);
        in_valid = 0;

        // Wait for two cycles of transmission
        @(posedge clk);
        if (mac_tvalid && mac_tdata[63:56] == 8'h42) 
            $display("PASS: Order Header Correct (BUY)");
        
        @(posedge clk);
        if (mac_tvalid && mac_tlast && mac_tdata[63:32] == 10050)
            $display("PASS: Order Payload Correct (Price)");

        #50;
        $finish;
    end

    initial begin
        $dumpfile("/Users/shreejitverma/Documents/GitHub/ultra-hft-project/apps/fpga-sim/waveforms/tb_encoder_egress.vcd");
        $dumpvars(0, tb_encoder_egress);
    end
endmodule
