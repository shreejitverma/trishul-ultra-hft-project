`timescale 1ns / 1ps

module tb_itch_decoder;

    // Inputs
    reg clk;
    reg rst;
    reg [63:0] s_axis_tdata;
    reg s_axis_tvalid;
    reg s_axis_tlast;

    // Outputs
    wire s_axis_tready;
    wire tick_valid;
    wire tick_type;
    wire [63:0] tick_oid;
    wire tick_side;
    wire [31:0] tick_qty;
    wire [31:0] tick_price;

    // Instantiate UUT
    itch_decoder #(
        .DATA_W(64)
    ) uut (
        .clk(clk), 
        .rst(rst), 
        .s_axis_tdata(s_axis_tdata), 
        .s_axis_tvalid(s_axis_tvalid), 
        .s_axis_tlast(s_axis_tlast), 
        .s_axis_tready(s_axis_tready), 
        .tick_valid(tick_valid), 
        .tick_type(tick_type), 
        .tick_oid(tick_oid), 
        .tick_side(tick_side), 
        .tick_qty(tick_qty), 
        .tick_price(tick_price)
    );

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        rst = 1;
        s_axis_tdata = 0;
        s_axis_tvalid = 0;
        s_axis_tlast = 0;
        
        #100;
        rst = 0;
        #20;
        
        // Emulate sending 5 words (40 bytes) of an ITCH message
        // Just filling the buffer to trigger the validity logic
        repeat(5) begin
            @(posedge clk);
            s_axis_tvalid = 1;
            s_axis_tdata = 64'hAA_BB_CC_DD_11_22_33_44; // Dummy data
        end
        
        @(posedge clk);
        s_axis_tvalid = 0;
        
        #50;
        if (tick_valid) 
            $display("PASS: Tick Generated");
        else 
            $display("FAIL: No Tick Generated");
            
        $finish;
    end
      
    initial begin
        $dumpfile("/Users/shreejitverma/Documents/GitHub/ultra-hft-project/apps/fpga-sim/waveforms/tb_itch_decoder.vcd");
        $dumpvars(0, tb_itch_decoder);
    end
endmodule
