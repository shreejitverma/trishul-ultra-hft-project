`timescale 1ns / 1ps

module tb_book2;

    reg clk;
    reg rst;
    reg tick_valid;
    reg tick_type;
    reg tick_side;
    reg [31:0] tick_qty;
    reg [31:0] tick_price;

    wire [31:0] bid_px0;
    wire [31:0] bid_sz0;
    wire [31:0] ask_px0;
    wire [31:0] ask_sz0;

    book2 uut (
        .clk(clk), .rst(rst),
        .tick_valid(tick_valid), .tick_type(tick_type),
        .tick_side(tick_side), .tick_qty(tick_qty), .tick_price(tick_price),
        .bid_px0(bid_px0), .bid_sz0(bid_sz0), .ask_px0(ask_px0), .ask_sz0(ask_sz0)
    );

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        rst = 1;
        tick_valid = 0;
        tick_type = 0;
        tick_side = 0;
        tick_qty = 0;
        tick_price = 0;
        
        #100;
        rst = 0;
        #20;

        // 1. Add a BID at 100.00 (10000 units)
        @(posedge clk);
        tick_valid = 1; tick_type = 0; tick_side = 1; tick_qty = 10; tick_price = 10000;
        @(posedge clk);
        tick_valid = 0;
        
        // 2. Add an ASK at 100.10 (10010 units)
        @(posedge clk);
        tick_valid = 1; tick_type = 0; tick_side = 0; tick_qty = 5; tick_price = 10010;
        @(posedge clk);
        tick_valid = 0;

        // 3. Add a better BID at 100.05
        @(posedge clk);
        tick_valid = 1; tick_type = 0; tick_side = 1; tick_qty = 20; tick_price = 10005;
        @(posedge clk);
        tick_valid = 0;

        #50;
        if (bid_px0 == 10005 && ask_px0 == 10010)
            $display("PASS: Order Book BBO Correctly Updated");
        else
            $display("FAIL: Order Book Mismatch. Bid: %d, Ask: %d", bid_px0, ask_px0);

        $finish;
    end

endmodule
