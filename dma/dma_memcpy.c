#include <linux/init.h>
#include <linux/module.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

struct dma_ctx {
	struct dma_chan *chan;
	dma_addr_t src_pa;
	void *src_va;
	dma_addr_t dst_pa;
	void *dst_va;
	int buf_sz;
};

struct dma_ctx *ctx;

static int dma_chan_init(struct dma_ctx *ctx)
{
	dma_cap_mask_t mask;

	pr_info("%s\n", __func__);

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE | DMA_PRIVATE, mask);

	ctx->chan = dma_request_channel(mask, NULL, NULL);
	if (!ctx->chan) {
		printk("dma_request_channel() failed\n");
		return -ENODEV;
	}

	pr_info("DMA channel name: %s\n", dma_chan_name(ctx->chan));
	return 0;
}

static int dma_mem_alloc(struct dma_ctx *ctx)
{
	struct dma_device *dma_dev = ctx->chan->device;

	pr_info("%s\n", __func__);

	ctx->buf_sz = 1024;

	ctx->src_va = dma_alloc_coherent(dma_dev->dev, ctx->buf_sz, &ctx->src_pa, GFP_KERNEL);
	if (!ctx->src_va)
		return -ENOMEM;

	ctx->dst_va = dma_alloc_coherent(dma_dev->dev, ctx->buf_sz, &ctx->dst_pa, GFP_KERNEL);
	if (!ctx->dst_va)
		return -ENOMEM;

	pr_info("src_va: %px, src_pa: %llx, dst_va: %px, dst_pa: %llx\n", ctx->src_va, ctx->src_pa, ctx->dst_va,
		ctx->dst_pa);

	return 0;
}

static int dma_mem_copy(struct dma_ctx *ctx)
{
	struct dma_async_tx_descriptor *tx;
	dma_cookie_t cookie;

	pr_info("%s\n", __func__);

	tx = dmaengine_prep_dma_memcpy(ctx->chan, ctx->dst_pa, ctx->src_pa, ctx->buf_sz, DMA_MEM_TO_MEM);
	if (!tx) {
		pr_info("dmaengine_prep_dma_memcpy() failed\n");
		return -EIO;
	}

	cookie = dmaengine_submit(tx);
	if (dma_submit_error(cookie)) {
		pr_info("dmaengine_submit() failed\n");
		return -EIO;
	}

	return dma_sync_wait(tx->chan, cookie);
}

static int __init dma_init(void)
{
	int ret;

	pr_info("%s\n", __func__);

	ctx = kzalloc(sizeof(struct dma_ctx), GFP_KERNEL);

	ret = dma_chan_init(ctx);
	if (ret < 0)
		return ret;

	ret = dma_mem_alloc(ctx);
	if (ret < 0)
		goto alloc_err;

	ret = dma_mem_copy(ctx);
	if (ret != DMA_COMPLETE) {
		pr_info("Error on DMA transfer: %d\n", ret);
		goto alloc_err;
	}

	pr_info("DMA transfer has completed!\n");
	return 0;

alloc_err:

	if (ctx->src_va)
		dma_free_coherent(ctx->chan->device->dev, ctx->buf_sz, ctx->src_va, ctx->src_pa);

	if (ctx->dst_va)
		dma_free_coherent(ctx->chan->device->dev, ctx->buf_sz, ctx->dst_va, ctx->dst_pa);

	dma_release_channel(ctx->chan);
	return ret;
}

static void __exit dma_exit(void)
{
	pr_info("%s\n", __func__);

	dmaengine_terminate_sync(ctx->chan);

	if (ctx->src_va)
		dma_free_coherent(ctx->chan->device->dev, ctx->buf_sz, ctx->src_va, ctx->src_pa);

	if (ctx->dst_va)
		dma_free_coherent(ctx->chan->device->dev, ctx->buf_sz, ctx->dst_va, ctx->dst_pa);

	dma_release_channel(ctx->chan);
}

module_init(dma_init);
module_exit(dma_exit);

MODULE_LICENSE("GPL");
